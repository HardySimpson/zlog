/* Copyright (c) Hardy Simpson
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __zlog_rotater_h
#define __zlog_rotater_h

#include "zc_defs.h"
#include "lockfile.h"

typedef struct zlog_rotater_s {
	pthread_mutex_t lock_mutex;
	char *lock_file;
	LOCK_FD lock_fd;

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
		char *archive_path, long archive_max_size, int archive_max_count);

void zlog_rotater_profile(zlog_rotater_t *a_rotater, int flag);

#endif
