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

#ifndef __zlog_thread_table_h
#define __zlog_thread_table_h

#include "zc_defs.h"
#include "thread.h"


zc_hashtable_t *zlog_thread_table_new(void);
void zlog_thread_table_del(zc_hashtable_t *threads);
void zlog_thread_table_profile(zc_hashtable_t *threads, int flag);

int zlog_thread_table_rebuild_msg_buf(zc_hashtable_t * threads,
			size_t buf_size_min,
			size_t buf_size_max);

zlog_thread_t *zlog_thread_table_get_thread(zc_hashtable_t * threads, pthread_t tid);
zlog_thread_t *zlog_thread_table_new_thread(zc_hashtable_t * threads, size_t buf_size_min,
				    size_t buf_size_max);

#endif
