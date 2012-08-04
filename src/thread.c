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
	zc_profile(flag, "--thread[%p][%p][%p][%p,%p,%p,%p,%p]--",
			a_thread,
			a_thread->mdc,
			a_thread->event,
			a_thread->pre_path_buf,
			a_thread->path_buf,
			a_thread->archive_path_buf,
			a_thread->pre_msg_buf,
			a_thread->msg_buf);

	zlog_mdc_profile(a_thread->mdc, flag);
	zlog_event_profile(a_thread->event, flag);
	zlog_buf_profile(a_thread->pre_path_buf, flag);
	zlog_buf_profile(a_thread->path_buf, flag);
	zlog_buf_profile(a_thread->archive_path_buf, flag);
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
	if (a_thread->archive_path_buf)
		zlog_buf_del(a_thread->archive_path_buf);
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
	zlog_thread_t *a_thread;

	a_thread = calloc(1, sizeof(zlog_thread_t));
	if (!a_thread) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_thread->mdc = zlog_mdc_new();
	if (!a_thread->mdc) {
		zc_error("zlog_mdc_new fail");
		goto err;
	}

	a_thread->event = zlog_event_new();
	if (!a_thread->event) {
		zc_error("zlog_event_new fail");
		goto err;
	}

	a_thread->pre_path_buf = zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->pre_path_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	a_thread->path_buf = zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->path_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	a_thread->archive_path_buf = zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->archive_path_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	a_thread->pre_msg_buf = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->pre_msg_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	a_thread->msg_buf =
	    zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->msg_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}


	//zlog_thread_profile(a_thread, ZC_DEBUG);
	return a_thread;
err:
	zlog_thread_del(a_thread);
	return NULL;
}

/*******************************************************************************/
int zlog_thread_update_msg_buf(zlog_thread_t * a_thread, size_t buf_size_min, size_t buf_size_max)
{
	zc_assert(a_thread, -1);

	/* 1st, mv msg_buf msg_buf_backup */
	if (a_thread->pre_msg_buf_backup) zlog_buf_del(a_thread->pre_msg_buf_backup);
	if (a_thread->msg_buf_backup) zlog_buf_del(a_thread->msg_buf_backup);
	a_thread->pre_msg_buf_backup = a_thread->pre_msg_buf;
	a_thread->msg_buf_backup = a_thread->msg_buf;


	/* 2nd, gen new buf */
	a_thread->pre_msg_buf = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->pre_msg_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	a_thread->msg_buf = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
	if (!a_thread->msg_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	return 0;
err:
	if (a_thread->pre_msg_buf) zlog_buf_del(a_thread->pre_msg_buf);
	if (a_thread->msg_buf) zlog_buf_del(a_thread->msg_buf);
	a_thread->pre_msg_buf = NULL;
	a_thread->msg_buf = NULL;
	return -1;
}

void zlog_thread_commit_msg_buf(zlog_thread_t * a_thread)
{
	zc_assert(a_thread, );
	if (!a_thread->pre_msg_buf_backup && !a_thread->msg_buf_backup) {
		zc_warn("backup is null, never update before");
		return;
	}

	if (a_thread->pre_msg_buf_backup) zlog_buf_del(a_thread->pre_msg_buf_backup);
	a_thread->pre_msg_buf_backup = NULL;
	if (a_thread->msg_buf_backup) zlog_buf_del(a_thread->msg_buf_backup);
	a_thread->msg_buf_backup = NULL;
	return;
}

void zlog_thread_rollback_msg_buf(zlog_thread_t * a_thread)
{
	zc_assert(a_thread,);
	if (!a_thread->pre_msg_buf_backup || !a_thread->msg_buf_backup) {
		zc_warn("backup is null, never update before");
		return;
	}

	if (a_thread->pre_msg_buf) {
		/* update success */
		zlog_buf_del(a_thread->pre_msg_buf);
		a_thread->pre_msg_buf = a_thread->pre_msg_buf_backup;
		a_thread->pre_msg_buf_backup = NULL;
	} else {
		/* update fail */
		a_thread->pre_msg_buf = a_thread->pre_msg_buf_backup;
		a_thread->pre_msg_buf_backup = NULL;
	}

	if (a_thread->msg_buf) {
		/* update success */
		zlog_buf_del(a_thread->msg_buf);
		a_thread->msg_buf = a_thread->msg_buf_backup;
		a_thread->msg_buf_backup = NULL;
	} else {
		/* update fail */
		a_thread->msg_buf = a_thread->msg_buf_backup;
		a_thread->msg_buf_backup = NULL;
	}
	return;
}

/*******************************************************************************/
