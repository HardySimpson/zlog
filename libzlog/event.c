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
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>

#include "zc_defs.h"
#include "event.h"

void zlog_event_profile(zlog_event_t * a_event, int flag)
{
	zc_assert(a_event,);
	zc_profile(flag, "---event[%p][%s,%s][%s,%ld][%p,%s][%ld,%ld][%ld,%ld]---",
			a_event,
			a_event->category_name, a_event->host_name,
			a_event->file, a_event->line,
			a_event->hex_buf, a_event->str_format,	
			a_event->time_stamp.tv_sec, a_event->time_stamp.tv_usec,
			(long)a_event->pid, (long)a_event->tid);
	return;
}

/*******************************************************************************/

void zlog_event_del(zlog_event_t * a_event)
{
	zc_assert(a_event,);
	free(a_event);
	zc_debug("zlog_event_del[%p]", a_event);
	return;
}

zlog_event_t *zlog_event_new(void)
{
	int rc = 0;
	zlog_event_t *a_event;

	a_event = calloc(1, sizeof(zlog_event_t));
	if (!a_event) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	/*
	 * at the zlog_init we gethostname,
	 * u don't always change your hostname, eh?
	 */
	rc = gethostname(a_event->host_name, sizeof(a_event->host_name) - 1);
	if (rc) {
		zc_error("gethostname fail, rc[%d], errno[%d]", rc, errno);
		goto zlog_event_new_exit;
	}

	a_event->host_name_len = strlen(a_event->host_name);

	/* tid is bound to a_event
	 * as in whole lifecycle event persists
	 * even fork to oth pid, tid not change
	 */
	a_event->tid = pthread_self();

      zlog_event_new_exit:
	if (rc) {
		zlog_event_del(a_event);
		return NULL;
	} else {
		zlog_event_profile(a_event, ZC_DEBUG);
		return a_event;
	}
}

/*******************************************************************************/
void zlog_event_set(zlog_event_t * a_event,
			char *category_name, size_t category_name_len,
			const char *file, size_t file_len, const char *func, size_t func_len,  long line, int level,
			const void *hex_buf, size_t hex_buf_len, const char *str_format, va_list str_args,
			int generate_cmd)
{
	/*
	 * category_name point to zlog_category_output's category.name
	 */
	a_event->category_name = category_name;
	a_event->category_name_len = category_name_len;

	a_event->file = (char *) file;
	a_event->file_len = file_len;
	a_event->func = (char *) func;
	a_event->func_len = func_len;
	a_event->line = line;
	a_event->level = level;

	a_event->generate_cmd = generate_cmd;
	switch (generate_cmd) {
	case ZLOG_HEX:
		a_event->hex_buf = hex_buf;
		a_event->hex_buf_len = hex_buf_len;
		break;
	case ZLOG_FMT:
		a_event->str_format = str_format;
		va_copy(a_event->str_args, str_args);
		break;
	default:
		break;
	}

	/* pid should fetch eveytime, as no one knows,
	 * when does user fork his process
	 * so clean here, and fetch at spec.c
	 */
	a_event->pid = (pid_t) 0;

	/* in a event's life cycle, time will be get when spec need,
	 * and keep unchange though all event's life cycle
	 */
	memset(&(a_event->time_stamp), 0x00, sizeof(a_event->time_stamp));
	return;
}
