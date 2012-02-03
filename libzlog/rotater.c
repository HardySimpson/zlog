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

#include "rotater.h"
#include "zc_defs.h"

zlog_rotater_t zlog_env_rotater;
static int zlog_rotater_debug(zlog_rotater_t * a_rot);

/*******************************************************************************/
typedef struct {
	int index;
	char path[MAXLEN_PATH + 1];
} zlog_rotater_onefile_t;

static zlog_rotater_onefile_t *zlog_rotater_onefile_new(char *base_file_path,
							char *real_file_path)
{
	int rc = 0;
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
	nwrite =
	    snprintf(a_file->path, sizeof(a_file->path), "%s", real_file_path);
	if (nwrite < 0 || nwrite >= sizeof(a_file->path)) {
		zc_error("not enough space for a_file->path, nwrite=[%d]",
			 nwrite);
		rc = -1;
		goto zlog_rotater_onefile_new_exit;
	}

	/* compare and set index, need check */
	if (STRCMP(base_file_path, !=, real_file_path)) {

		/* scan aa.123 to index=123 */
		sscanf(real_file_path + strlen(base_file_path) + 1, "%d",
		       &(a_file->index));

		/* check, we do not need file like aa.001 */
		memset(&tmp, 0x00, sizeof(tmp));
		nwrite =
		    snprintf(tmp, sizeof(tmp), "%s.%d", base_file_path,
			     a_file->index);
		if (nwrite < 0 || nwrite >= sizeof(tmp)) {
			zc_error("not enough space for tmp, nwrite=[%d]",
				 nwrite);
			rc = -1;
			goto zlog_rotater_onefile_new_exit;
		}

		if (STRCMP(real_file_path, !=, tmp)) {
			zc_error
			    ("real_file_path[%s] format is not aa.1, maybe aa.001",
			     real_file_path);
			rc = -1;
			goto zlog_rotater_onefile_new_exit;
		}		/* else ok that's right */
	} else {
		a_file->index = 0;
	}

      zlog_rotater_onefile_new_exit:
	if (rc) {
		free(a_file);
		return NULL;
	} else {
		zc_debug("new onefile at [%p]", a_file);
		zc_debug("a_file->path[%s]", a_file->path);
		return a_file;
	}
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
int zlog_rotater_init(zlog_rotater_t * a_rot, char *lock_file)
{
	int rc = 0;
	int fd = 0;

	zc_assert_debug(a_rot, -1);
	zc_assert_debug(lock_file, -1);

	rc = pthread_mutex_init(&(a_rot->mlock), NULL);
	if (rc) {
		zc_error("pthread_mutex_init fail, errno[%d]", errno);
		return -1;
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
		return -1;
	}

	a_rot->lock_fd = fd;

	zlog_rotater_debug(a_rot);
	return 0;
}

int zlog_rotater_update(zlog_rotater_t * a_rot, char *lock_file)
{
	int rc = 0;
	int fd = 0;
	const char *lf;

	zc_assert_debug(a_rot, -1);
	zc_assert_debug(lock_file, -1);

	if (a_rot->lock_fd) {
		rc = close(a_rot->lock_fd);
		if (rc) {
			zc_error("close fail, errno[%d]", errno);
		}
	}

	if (STRCMP(lock_file, ==, "")) {
		lf = "/tmp/zlog.lock";
	} else {
		lf = lock_file;
	}

	fd = open(lf, O_RDWR | O_CREAT,
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd < 0) {
		zc_error("open file[%s] fail, errno[%d]", lf, errno);
		return -1;
	}

	a_rot->lock_fd = fd;

	zlog_rotater_debug(a_rot);
	return 0;
}

void zlog_rotater_fini(zlog_rotater_t * a_rot)
{
	int rc = 0;
	zc_assert_debug(a_rot,);

	rc = pthread_mutex_destroy(&(a_rot->mlock));
	if (rc) {
		zc_error("pthread_mutex_destroy fail, errno[%d]", errno);
	}

	if (a_rot->lock_fd) {
		rc = close(a_rot->lock_fd);
		if (rc) {
			zc_error("close fail, errno[%d]", errno);
		}
	}

	memset(a_rot, 0x00, sizeof(*a_rot));
	return;
}

/*******************************************************************************/

static int zlog_rotater_trylock(zlog_rotater_t * a_rot)
{
	int rc = 0;
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;

	rc = pthread_mutex_trylock(&(a_rot->mlock));
	if (rc) {
		if (errno == EBUSY) {
			zc_debug
			    ("pthread_mutex_trylock fail, as mlock is locked by other threads");
		} else {
			zc_error("pthread_mutex_trylock fail, errno[%d]",
				 errno);
		}
		return -1;
	}

	rc = fcntl(a_rot->lock_fd, F_SETLK, &fl);
	if (rc == -1) {
		if (errno == EAGAIN || errno == EACCES) {
			/* lock by other process, that's right, go on */
			/* EAGAIN on linux */
			/* EACCES on AIX */
			zc_debug
			    ("fcntl lock fail, as file is lock by other process");
		} else {
			zc_error("lock fd[%d] fail, errno[%d]", a_rot->lock_fd,
				 errno);
		}
		rc = pthread_mutex_unlock(&(a_rot->mlock));
		if (rc) {
			zc_error("pthread_mutex_unlock fail, errno[%d]", errno);
		}
		return -1;
	}

	return 0;
}

static int zlog_rotater_unlock(zlog_rotater_t * a_rot)
{
	int rc = 0;
	int rd = 0;
	struct flock fl;

	fl.l_type = F_UNLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;

	rc = fcntl(a_rot->lock_fd, F_SETLK, &fl);
	if (rc == -1) {
		rd = -1;
		zc_error("unlock fd[%s] fail, errno[%d]", a_rot->lock_fd,
			 errno);
	}

	rc = pthread_mutex_unlock(&(a_rot->mlock));
	if (rc) {
		rd = -1;
		zc_error("pthread_mutext_unlock fail, errno[%d]", errno);
	}

	return rd;
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

int zlog_rotater_lsmv(char *base_file_path)
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

	files =
	    zc_arraylist_new((zc_arraylist_del_fn) zlog_rotater_onefile_del);
	if (!files) {
		zc_error("zc_arraylist_new fail");
		return -1;
	}

	/* scan file which is aa.[0-9]* and aa */
	rc = glob(base_file_path, GLOB_ERR | GLOB_MARK | GLOB_NOSORT, NULL,
		  &glob_buf);
	if (rc) {
		zc_error("glob err 1, rc=[%d], errno[%d]", rc, errno);
		rc = -1;
		goto zlog_rotater_lsmv_exit;
	}

	memset(&tmp, 0x00, sizeof(tmp));
	nwrite = snprintf(tmp, sizeof(tmp), "%s.[0-9]*", base_file_path);
	if (nwrite < 0 || nwrite >= sizeof(tmp)) {
		zc_error("not enough space for tmp, nwrite=[%d]", nwrite);
		rc = -1;
		goto zlog_rotater_lsmv_exit;
	}

	rc = glob(tmp, GLOB_ERR | GLOB_MARK | GLOB_NOSORT | GLOB_APPEND, NULL,
		  &glob_buf);
	if (rc != 0 && rc != GLOB_NOMATCH) {
		zc_error("glob err 2, rc=[%d], errno[%d]", rc, errno);
		rc = -1;
		goto zlog_rotater_lsmv_exit;
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
			rc = -1;
			goto zlog_rotater_lsmv_exit;
		}

		rc = zc_arraylist_sortadd(files,
					  (zc_arraylist_cmp_fn)
					  zlog_rotater_onefile_cmp, a_file);
		if (rc) {
			zc_error("zc_arraylist_sortadd fail");
			rc = -1;
			goto zlog_rotater_lsmv_exit;
		}
	}

	for (i = zc_arraylist_len(files) - 1; i > -1; i--) {
		a_file = zc_arraylist_get(files, i);
		if (!a_file) {
			zc_error("zc_arraylist_get fail");
			rc = -1;
			goto zlog_rotater_lsmv_exit;
		}

		memset(&tmp, 0x00, sizeof(tmp));
		nwrite =
		    snprintf(tmp, sizeof(tmp), "%s.%d", base_file_path,
			     a_file->index + 1);
		if (nwrite < 0 || nwrite >= sizeof(tmp)) {
			zc_error("not enough space for tmp, nwrite=[%d]",
				 nwrite);
			rc = -1;
			goto zlog_rotater_lsmv_exit;
		}

		rc = rename(a_file->path, tmp);
		if (rc) {
			zc_error("rename fail, errno[%d]", errno);
			rc = -1;
			goto zlog_rotater_lsmv_exit;
		}
	}

      zlog_rotater_lsmv_exit:

	globfree(&glob_buf);
	zc_arraylist_del(files);

	return rc;
}

int zlog_rotater_rotate(zlog_rotater_t * a_rot, char *file_path,
			long file_maxsize, size_t msg_len)
{
	int rc = 0;
	int rd = 0;
	struct stat info;

	zc_assert_debug(a_rot, -1);
	zc_assert_debug(file_path, -1);

	if (msg_len > file_maxsize) {
		zc_debug("one msg's len[%ld] > file_maxsize[%ld], no rotate",
			 (long)msg_len, file_maxsize);
		return 0;
	}

	rc = stat(file_path, &info);
	if (rc) {
		zc_error("stat [%s] fail, errno[%d]", file_path, errno);
		return -1;
	} else {
		if (info.st_size + msg_len < file_maxsize) {
			/* file not so big, return */
			return 0;
		}
	}

	rd = zlog_rotater_trylock(a_rot);
	if (rd) {
		zc_error("zlog_rotater_trylock fail");
		return 0;
	}

	rc = stat(file_path, &info);
	if (rc) {
		rc = -1;
		zc_error("stat [%s] fail, errno[%d]", file_path, errno);
		goto zlog_rotater_rotate_exit;
	}

	if (info.st_size + msg_len <= file_maxsize) {
		/* file not so big, return */
		rc = 0;
		goto zlog_rotater_rotate_exit;
	}

	/* begin list and move files */
	rc = zlog_rotater_lsmv(file_path);
	if (rc) {
		zc_error("zlog_rotater_file_ls_mv fail, return");
		rc = -1;
		goto zlog_rotater_rotate_exit;
	} else if (rc == 0) {
		zc_debug("zlog_rotater_file_ls_mv success");
	}

      zlog_rotater_rotate_exit:
	/* unlock file */
	rd = zlog_rotater_unlock(a_rot);
	if (rd) {
		zc_error("zlog_rotater_unlock fail");
	}

	return rc;
}

/*******************************************************************************/
static int zlog_rotater_debug(zlog_rotater_t * a_rot)
{
	zc_debug("---a_rot start[%p]---", a_rot);
	zc_debug("a_rot->mlock[%p]", a_rot->mlock);
	zc_debug("a_rot->lock_fd[%d]", a_rot->lock_fd);
	return 0;
}
