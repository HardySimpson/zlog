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
	zc_debug("first in thread[%ld]", a_event->tid);

      zlog_event_new_exit:
	if (rc) {
		zlog_event_del(a_event);
		return NULL;
	} else {
		return a_event;
	}
}

void zlog_event_del(zlog_event_t * a_event)
{
	zc_assert(a_event,);

	zc_debug("free a_event at[%p]", a_event);
	free(a_event);
	return;
}

void zlog_event_refresh(zlog_event_t * a_event,
			char *category_name, size_t * category_name_len,
			char *file, long line, int priority,
			char *hex_buf, long hex_buf_len, char *str_format,
			va_list str_args, int generate_cmd)
{
	zc_assert(a_event,);

	/*
	 * category_name point to zlog_category_output's category.name
	 */
	a_event->category_name = category_name;
	a_event->category_name_len = category_name_len;

	a_event->file = file;
	a_event->line = line;
	a_event->priority = priority;

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

	memset(&(a_event->time_stamp), 0x00, sizeof(a_event->time_stamp));
	return;
}
