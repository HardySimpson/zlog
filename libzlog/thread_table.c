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

#include <errno.h>
#include "zc_defs.h"
#include "thread_table.h"


void zlog_thread_table_profile(zc_hashtable_t * threads, int flag)
{
	zc_hashtable_entry_t *a_entry;
	zlog_thread_t *a_thread;

	zc_assert(threads,);
	zc_profile(flag, "--thread_table[%p]--", threads);
	zc_hashtable_foreach(threads, a_entry) {
		a_thread = (zlog_thread_t *) a_entry->value;
		zlog_thread_profile(a_thread, flag);
	}
	return;
}

/*******************************************************************************/
void zlog_thread_table_del(zc_hashtable_t * threads)
{
	zc_assert(threads,);
	zc_hashtable_del(threads);
	zc_debug("zlog_thread_table_del[%p]", threads);
	return;
}

zc_hashtable_t *zlog_thread_table_new(void)
{
	zc_hashtable_t *threads;

	threads = zc_hashtable_new(20,
			 (zc_hashtable_hash_fn) zc_hashtable_tid_hash,
			 (zc_hashtable_equal_fn) zc_hashtable_tid_equal,
			 NULL, (zc_hashtable_del_fn) zlog_thread_del);
	if (!threads) {
		zc_error("zc_hashtable_new fail");
		return NULL;
	} else {
		zlog_thread_table_profile(threads, ZC_DEBUG);
		return threads;
	}
}

/*******************************************************************************/
int zlog_thread_table_rebuild_msg_buf(zc_hashtable_t * threads,
			size_t buf_size_min,
			size_t buf_size_max)
{
	int rc = 0;
	zc_hashtable_entry_t *a_entry;
	zlog_thread_t *a_thread;

	zc_assert(threads, -1);
	zc_hashtable_foreach(threads, a_entry) {
		a_thread = (zlog_thread_t *) a_entry->value;
		rc = zlog_thread_rebuild_msg_buf(a_thread, buf_size_min, buf_size_max);
		if (rc) {
			zc_error("zlog_thread_rebuild_msg_buf fail");
			return -1;
		}
	}

	return 0;
}

zlog_thread_t *zlog_thread_table_get_thread(zc_hashtable_t * threads, pthread_t tid)
{
	zlog_thread_t *a_thread;

	a_thread = zc_hashtable_get(threads, (void *)&tid);
	if (!a_thread) {
		zc_debug("thread[%ld] not found, maybe not create", tid);
		return NULL;
	} else {
		return a_thread;
	}
}

zlog_thread_t *zlog_thread_table_new_thread(zc_hashtable_t * threads,
				size_t buf_size_min, size_t buf_size_max)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	a_thread = zlog_thread_new(buf_size_min, buf_size_max);
	if (!a_thread) {
		zc_error("zlog_thread_new fail");
		return NULL;
	}

	rc = zc_hashtable_put(threads,
			(void *)&(a_thread->event->tid),
			(void *)a_thread);
	if (rc) {
		zc_error("zc_hashtable_put fail");
		goto zlog_thread_table_new_exit;
	}

      zlog_thread_table_new_exit:
	if (rc) {
		zlog_thread_del(a_thread);
		return NULL;
	} else {
		return a_thread;
	}
}


/*******************************************************************************/
