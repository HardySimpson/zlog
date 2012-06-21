/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * The zlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The zlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the zlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "zc_defs.h"
#include "rotater.h"

struct zlog_rotater_s {
	pthread_mutex_t lock_mutex;
	char *lock_file;
	int lock_fd;
};

void zlog_rotater_profile(zlog_rotater_t * a_rotater, int flag)
{
	zc_assert(a_rotater,);
	zc_profile(flag, "--rotater[%p][%p,%s,%d]--",
		a_rotater,
		&(a_rotater->lock_mutex),
		a_rotater->lock_file,
		a_rotater->lock_fd);
	return;
}

/*******************************************************************************/
void zlog_rotater_del(zlog_rotater_t *a_rotater)
{
	zc_assert(a_rotater,);

	if (a_rotater->lock_fd) {
		if (close(a_rotater->lock_fd)) {
			zc_error("close fail, errno[%d]", errno);
		}
	}

	if (pthread_mutex_destroy(&(a_rotater->lock_mutex))) {
		zc_error("pthread_mutex_destroy fail, errno[%d]", errno);
	}

	free(a_rotater);
	zc_debug("zlog_rotater_del[%p]", a_rotater);
	return;
}

zlog_rotater_t *zlog_rotater_new(char *lock_file)
{
	int fd = 0;
	zlog_rotater_t *a_rotater;

	zc_assert(lock_file, NULL);

	a_rotater = calloc(1, sizeof(zlog_rotater_t));
	if (!a_rotater) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	if (pthread_mutex_init(&(a_rotater->lock_mutex), NULL)) {
		zc_error("pthread_mutex_init fail, errno[%d]", errno);
		free(a_rotater);
		return NULL;
	}

	/* depends on umask of the user here
	 * if user A create /tmp/zlog.lock 0600
	 * user B is unable to read /tmp/zlog.lock
	 * B has to choose another lock file except /tmp/zlog.lock
	 */
	fd = open(lock_file, O_RDWR | O_CREAT,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd < 0) {
		zc_error("open file[%s] fail, errno[%d]", lock_file, errno);
		goto err;
	}

	a_rotater->lock_fd = fd;
	a_rotater->lock_file = lock_file;

	zlog_rotater_profile(a_rotater, ZC_DEBUG);
	return a_rotater;
err:
	zlog_rotater_del(a_rotater);
	return NULL;
}


/*******************************************************************************/
typedef struct {
	int index;
	char path[MAXLEN_PATH + 1];
} zlog_rotater_onefile_t;

static zlog_rotater_onefile_t *zlog_rotater_onefile_new(char *base_file_path,
							char *real_file_path)
{
	int nwrite;
	char tmp[MAXLEN_PATH + 1];
	zlog_rotater_onefile_t *a_file;

	if (STRNCMP(base_file_path, !=, real_file_path, strlen(base_file_path))) {
		zc_error
		    ("real_file_path[%s] is not consist of base_file_path[%s]",
		     real_file_path, base_file_path);
		return NULL;
	}

	a_file = calloc(1, sizeof(zlog_rotater_onefile_t));
	if (!a_file) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	/* ok, set the path */
	nwrite = snprintf(a_file->path, sizeof(a_file->path), "%s", real_file_path);
	if (nwrite < 0 || nwrite >= sizeof(a_file->path)) {
		zc_error("not enough space for a_file->path, nwrite=[%d]", nwrite);
		goto err;
	}

	/* compare and set index, need check */
	if (STRCMP(base_file_path, !=, real_file_path)) {
		/* scan aa.123 to index=123 */
		sscanf(real_file_path + strlen(base_file_path) + 1, "%d",
		       &(a_file->index));

		/* check, we do not need file like aa.001 */
		memset(&tmp, 0x00, sizeof(tmp));
		nwrite = snprintf(tmp, sizeof(tmp), "%s.%d", base_file_path, a_file->index);
		if (nwrite < 0 || nwrite >= sizeof(tmp)) {
			zc_error("not enough space for tmp, nwrite=[%d]",
				 nwrite);
			goto err;
		}

		if (STRCMP(real_file_path, !=, tmp)) {
			zc_error
			    ("real_file_path[%s] format is not aa.1, maybe aa.001",
			     real_file_path);
			goto err;
		}		/* else ok that's right */
	} else {
		a_file->index = 0;
	}

	zc_debug("new onefile at [%p]", a_file);
	zc_debug("a_file->path[%s]", a_file->path);
	return a_file;
err:
	free(a_file);
	return NULL;
}

static void zlog_rotater_onefile_del(zlog_rotater_onefile_t * a_file)
{
	zc_debug("del onefile[%p]", a_file);
	zc_debug("a_file->path[%s]", a_file->path);
	free(a_file);
}

static int zlog_rotater_onefile_cmp(zlog_rotater_onefile_t * a_file_1,
				    zlog_rotater_onefile_t * a_file_2)
{
	if (a_file_1->index - a_file_2->index > 0)
		return 1;
	else if (a_file_1->index - a_file_2->index < 0)
		return -1;
	else
		return 0;
}

/*******************************************************************************/

static int zlog_rotater_trylock(zlog_rotater_t *a_rotater)
{
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;

	if (pthread_mutex_trylock(&(a_rotater->lock_mutex))) {
		if (errno == EBUSY) {
			zc_warn("pthread_mutex_trylock fail, as lock_mutex is locked by other threads");
		} else {
			zc_error("pthread_mutex_trylock fail, errno[%d]",
				 errno);
		}
		return -1;
	}

	if (fcntl(a_rotater->lock_fd, F_SETLK, &fl)) {
		if (errno == EAGAIN || errno == EACCES) {
			/* lock by other process, that's right, go on */
			/* EAGAIN on linux */
			/* EACCES on AIX */
			zc_warn("fcntl lock fail, as file is lock by other process");
		} else {
			zc_error("lock fd[%d] fail, errno[%d]", a_rotater->lock_fd, errno);
		}
		if (pthread_mutex_unlock(&(a_rotater->lock_mutex))) {
			zc_error("pthread_mutex_unlock fail, errno[%d]", errno);
		}
		return -1;
	}

	return 0;
}

static int zlog_rotater_unlock(zlog_rotater_t *a_rotater)
{
	int rc = 0;
	struct flock fl;

	fl.l_type = F_UNLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;

	if (fcntl(a_rotater->lock_fd, F_SETLK, &fl)) {
		rc = -1;
		zc_error("unlock fd[%s] fail, errno[%d]", a_rotater->lock_fd, errno);
	}

	if (pthread_mutex_unlock(&(a_rotater->lock_mutex))) {
		rc = -1;
		zc_error("pthread_mutext_unlock fail, errno[%d]", errno);
	}

	return rc;
}

#if 0

static int zlog_rotater_one_file_mv_max(zlog_rotater_one_file * a_file,
					char *base_file_path)
{
	char tmp[MAXLEN_PATH + 1];
	int nwrite;

	memset(&tmp, 0x00, sizeof(tmp));
	nwrite =
	    snprintf(tmp, sizeof(tmp), "%s.%d", base_file_path,
		     a_file->index + 1);
	if (nwrite < 0 || nwrite >= sizeof(tmp)) {
		zc_error("not enough space for tmp, nwrite=[%d]", nwrite);
		return -1;
	}

	if (rename(base_file_path, tmp)) {
		zc_error("rename failed, errno[%d]", errno);
		return -1;
	}

	return 0;
}
#endif

int zlog_rotater_lsmv(char *base_file_path, int file_max_count)
{
	int rc = 0;

	char tmp[MAXLEN_PATH + 1];
	glob_t glob_buf;
	size_t pathc;
	char **pathv;
	int nwrite;
	zc_arraylist_t *files = NULL;

	zlog_rotater_onefile_t *a_file;
	int i;

	memset(&tmp, 0x00, sizeof(tmp));
	nwrite = snprintf(tmp, sizeof(tmp), "%s.[0-9]*", base_file_path);
	if (nwrite < 0 || nwrite >= sizeof(tmp)) {
		zc_error("not enough space for tmp, nwrite=[%d]", nwrite);
		return -1;
	}

	/* scan file which is aa.[0-9]* and aa */
	rc = glob(base_file_path, GLOB_ERR | GLOB_MARK | GLOB_NOSORT, NULL, &glob_buf);
	if (rc) {
		zc_error("glob err 1, rc=[%d], errno[%d]", rc, errno);
		return -1;
	}

	files = zc_arraylist_new((zc_arraylist_del_fn) zlog_rotater_onefile_del);
	if (!files) {
		zc_error("zc_arraylist_new fail");
		goto err;
	}

	rc = glob(tmp, GLOB_ERR | GLOB_MARK | GLOB_NOSORT | GLOB_APPEND, NULL, &glob_buf);
	if (rc != 0 && rc != GLOB_NOMATCH) {
		zc_error("glob err 2, rc=[%d], errno[%d]", rc, errno);
		goto err;
	}

	pathv = glob_buf.gl_pathv;
	pathc = glob_buf.gl_pathc;

	/* check and find aa.max */
	for (; pathc-- > 0; pathv++) {
		/* omit dirs */
		if ((*pathv)[strlen(*pathv) - 1] == '/')
			continue;

		a_file = zlog_rotater_onefile_new(base_file_path, *pathv);
		if (!a_file) {
			zc_error("zlog_rotater_onefile_new fail");
			goto err;
		}

		rc = zc_arraylist_sortadd(files,
				  (zc_arraylist_cmp_fn)
				  zlog_rotater_onefile_cmp, a_file);
		if (rc) {
			zc_error("zc_arraylist_sortadd fail");
			goto err;
		}
	}

	for (i = zc_arraylist_len(files) - 1; i > -1; i--) {
		a_file = zc_arraylist_get(files, i);
		if (!a_file) {
			zc_error("zc_arraylist_get fail");
			goto err;
		}

		if (file_max_count > 0 && i >= file_max_count - 1) {
			/* remove file.3 >= 3*/
			rc = unlink(a_file->path);
			if (rc) {
				zc_error("unlink fail, errno[%d]", errno);
				goto err;
			}
			continue;
		}

		memset(&tmp, 0x00, sizeof(tmp));
		nwrite = snprintf(tmp, sizeof(tmp), "%s.%d", base_file_path, a_file->index + 1);
		if (nwrite < 0 || nwrite >= sizeof(tmp)) {
			zc_error("not enough space for tmp, nwrite=[%d]", nwrite);
			goto err;
		}

		if (rename(a_file->path, tmp)) {
			zc_error("rename fail, errno[%d]", errno);
			goto err;
		}
	}

	if (files) zc_arraylist_del(files);
	globfree(&glob_buf);
	return 0;
err:
	if (files) zc_arraylist_del(files);
	globfree(&glob_buf);
	return -1;
}

int zlog_rotater_rotate(zlog_rotater_t *a_rotater, 
		char *file_path, long file_max_size, int file_max_count,
		size_t msg_len)
{
	int rc = 0;
	struct stat info;

	zc_assert(file_path, -1);

	if (msg_len > file_max_size) {
		zc_debug("one msg's len[%ld] > file_max_size[%ld], no rotate",
			 (long)msg_len, file_max_size);
		return 0;
	}

	if (stat(file_path, &info)) {
		zc_warn("stat [%s] fail, errno[%d], maybe in rotating", file_path, errno);
		return 0;
	}

	/* file not so big, return */
	if (info.st_size + msg_len < file_max_size) return 0;

	if (zlog_rotater_trylock(a_rotater)) {
		zc_warn("zlog_rotater_trylock fail, maybe lock by other process or threads");
		return 0;
	}

	if (stat(file_path, &info)) {
		rc = -1;
		zc_error("stat [%s] fail, errno[%d]", file_path, errno);
		goto exit;
	}

	if (info.st_size + msg_len <= file_max_size) {
		/* file not so big,
		 * may alread rotate by oth process or thread,
		 * return */
		rc = 0;
		goto exit;
	}

	/* begin list and move files */
	rc = zlog_rotater_lsmv(file_path, file_max_count);
	if (rc) {
		zc_error("zlog_rotater_file_ls_mv [%s] fail, return", file_path);
		rc = -1;
		goto exit;
	} else if (rc == 0) {
		rc = 1;
		zc_debug("zlog_rotater_file_ls_mv success");
	}

exit:
	/* unlock file */
	if (zlog_rotater_unlock(a_rotater)) {
		zc_error("zlog_rotater_unlock fail");
	}

	return rc;
}

/*******************************************************************************/
