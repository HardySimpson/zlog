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

#ifndef __zlog_rotater_h
#define __zlog_rotater_h

#include "zc_defs.h"

typedef struct zlog_rotater_s {
	pthread_mutex_t lock_mutex;
	char *lock_file;
	int lock_fd;

	/* single-use members */
	char *base_path;			/* aa.log */
	char *archive_path;			/* aa.#5i.log */
	char glob_path[MAXLEN_PATH + 1];	/* aa.*.log */
	size_t num_start_len;			/* 3, offset to glob_path */
	size_t num_end_len;			/* 6, offset to glob_path */
	int num_width;				/* 5 */
	int mv_type;				/* ROLLING or SEQUENCE */
	int max_count;
	zc_arraylist_t *files;
} zlog_rotater_t;

zlog_rotater_t *zlog_rotater_new(char *lock_file);
void zlog_rotater_del(zlog_rotater_t *a_rotater);

/*
 * return
 * -1	fail
 * 0	no rotate, or rotate and success
 */
int zlog_rotater_rotate(zlog_rotater_t *a_rotater,
		char *base_path, size_t msg_len,
		char *archive_path, long archive_max_size, int archive_max_count,
		int *reopen_fd, int reopen_flags, unsigned int reopen_perms);

void zlog_rotater_profile(zlog_rotater_t *a_rotater, int flag);

#endif
