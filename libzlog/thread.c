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

#include <pthread.h>
#include <errno.h>

#include "zc_defs.h"
#include "event.h"
#include "buf.h"
#include "thread.h"
#include "mdc.h"

void zlog_thread_profile(zlog_thread_t * a_thread, int flag)
{
	zc_assert(a_thread,);
	zlog_profile(flag, "--thread[%p][%p][%p][%p,%p,%p,%p]--",
			a_thread,
			a_thread->mdc,
			a_thread->event,
			a_thread->pre_path_buf,
			a_thread->path_buf,
			a_thread->pre_msg_buf,
			a_thread->msg_buf);

	zlog_mdc_profile(a_thread->mdc, flag);
	zlog_event_profile(a_thread->event, flag);
	zlog_buf_profile(a_thread->pre_path_buf, flag);
	zlog_buf_profile(a_thread->path_buf, flag);
	zlog_buf_profile(a_thread->pre_msg_buf, flag);
	zlog_buf_profile(a_thread->msg_buf, flag);
	return;
}
/*******************************************************************************/
void zlog_thread_del(zlog_thread_t * a_thread)
{
	zc_assert(a_thread,);
	if (a_thread->mdc)
		zlog_mdc_del(a_thread->mdc);
	if (a_thread->event)
		zlog_event_del(a_thread->event);
	if (a_thread->pre_path_buf)
		zlog_buf_del(a_thread->pre_path_buf);
	if (a_thread->path_buf)
		zlog_buf_del(a_thread->path_buf);
	if (a_thread->pre_msg_buf)
		zlog_buf_del(a_thread->pre_msg_buf);
	if (a_thread->msg_buf)
		zlog_buf_del(a_thread->msg_buf);

	free(a_thread);
	zc_debug("zlog_thread_del[%p]", a_thread);
	return;
}

zlog_thread_t *zlog_thread_new(size_t buf_size_min, size_t buf_size_max)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	a_thread = calloc(1, sizeof(zlog_thread_t));
	if (!a_thread) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_thread->mdc = zlog_mdc_new();
	if (!a_thread->mdc) {
		zc_error("zlog_mdc_new fail");
		rc = -1;
		goto zlog_thread_init_exit;
	}

	a_thread->event = zlog_event_new();
	if (!a_thread->event) {
		zc_error("zlog_event_new fail");
		rc = -1;
		goto zlog_thread_init_exit;
	}

	a_thread->pre_path_buf =
	    zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->pre_path_buf) {
		zc_error("zlog_buf_new fail");
		rc = -1;
		goto zlog_thread_init_exit;
	}

	a_thread->path_buf =
	    zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->path_buf) {
		zc_error("zlog_buf_new fail");
		rc = -1;
		goto zlog_thread_init_exit;
	}

	a_thread->pre_msg_buf =
	    zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->pre_msg_buf) {
		zc_error("zlog_buf_new fail");
		rc = -1;
		goto zlog_thread_init_exit;
	}

	a_thread->msg_buf =
	    zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->msg_buf) {
		zc_error("zlog_buf_new fail");
		rc = -1;
		goto zlog_thread_init_exit;
	}

      zlog_thread_init_exit:

	if (rc) {
		zlog_thread_del(a_thread);
		return NULL;
	} else {
		zlog_thread_profile(a_thread, ZC_DEBUG);
		return a_thread;
	}
}

int zlog_thread_rebuild_msg_buf(zlog_thread_t * a_thread,
				size_t buf_size_min,
				size_t buf_size_max)
{
	int rc = 0;
	zlog_buf_t *pre_msg_buf = NULL;
	zlog_buf_t *msg_buf = NULL;

	zc_assert(a_thread,);
	pre_msg_buf = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->pre_msg_buf) {
		zc_error("zlog_buf_new fail");
		rc = -1;
		goto zlog_thread_rebuild_msg_buf_exit;
	}

	msg_buf = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->msg_buf) {
		zc_error("zlog_buf_new fail");
		rc = -1;
		goto zlog_thread_rebuild_msg_buf_exit;
	}

	if (a_thread->pre_msg_buf) zlog_buf_del(a_thread->pre_msg_buf);
	if (a_thread->msg_buf) zlog_buf_del(a_thread->msg_buf);
	a_thread->pre_msg_buf = pre_msg_buf;
	a_thread->msg_buf = msg_buf;

      zlog_thread_rebuild_msg_buf_exit:
	if (rc) {
		if (pre_msg_buf) zlog_buf_del(pre_msg_buf);
		if (msg_buf) zlog_buf_del(msg_buf);
		return -1;
	} else {
		zc_debug("update a thread at[%p]", a_thread);
		return 0;
	}
}

/*******************************************************************************/
