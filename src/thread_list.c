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
#include <pthread.h>
#include "zc_defs.h"
#include "thread_list.h"


void zlog_thread_list_profile(zc_arraylist_t * threads, int flag)
{
	int i;
	zlog_thread_t *a_thread;

	zc_assert(threads,);
	zc_profile(flag, "--thread_list[%p]--", threads);
	zc_arraylist_foreach(threads, i,  a_thread) {
		zlog_thread_profile(a_thread, flag);
	}
	return;
}

/*******************************************************************************/
void zlog_thread_list_del(zc_arraylist_t * threads)
{
	zc_assert(threads,);
	zc_arraylist_del(threads);
	zc_debug("zlog_thread_list_del[%p]", threads);
	return;
}

zc_arraylist_t *zlog_thread_list_new(void)
{
	zc_arraylist_t *threads;

	threads = zc_arraylist_new((zc_arraylist_del_fn)zlog_thread_del);
	if (!threads) {
		zc_error("zc_arraylist_new fail");
		return NULL;
	} else {
		zlog_thread_list_profile(threads, ZC_DEBUG);
		return threads;
	}
}

/*******************************************************************************/
int zlog_thread_list_update_msg_buf(zc_arraylist_t * threads, size_t buf_size_min, size_t buf_size_max)
{
	int i;
	zlog_thread_t *a_thread;

	zc_assert(threads, -1);
	zc_arraylist_foreach(threads, i, a_thread) {
		if (zlog_thread_update_msg_buf(a_thread, buf_size_min, buf_size_max)) {
			zc_error("zlog_thread_update_msg_buf fail, try rollback");
			return -1;
		}
	}

	return 0;
}

void zlog_thread_list_commit_msg_buf(zc_arraylist_t * threads)
{
	int i;
	zlog_thread_t *a_thread;

	zc_assert(threads,);
	zc_arraylist_foreach(threads, i, a_thread) {
		zlog_thread_commit_msg_buf(a_thread);
	}
	return;
}

void zlog_thread_list_rollback_msg_buf(zc_arraylist_t * threads)
{
	int i;
	zlog_thread_t *a_thread;

	zc_assert(threads,);
	zc_arraylist_foreach(threads, i, a_thread) {
		zlog_thread_rollback_msg_buf(a_thread);
	}
	return;
}

/*******************************************************************************/
zlog_thread_t *zlog_thread_list_new_thread(zc_arraylist_t * threads, pthread_key_t key,
		size_t buf_size_min, size_t buf_size_max)
{
	int rc;
	zlog_thread_t *a_thread; 
	a_thread = zlog_thread_new(buf_size_min, buf_size_max);
	if (!a_thread) {
		zc_error("zlog_thread_new fail");
		return NULL;
	}

	if (zc_arraylist_add(threads, a_thread)) {
		zc_error("zc_arraylist_put fail");
		goto err;
	}

	rc = pthread_setspecific(key, a_thread);
	if (rc) {
		zc_error("pthread_setspecific fail, rc[%d]");
		goto err;
	}

	return a_thread;
err:
	zlog_thread_del(a_thread);
	return NULL;
}
/*******************************************************************************/
