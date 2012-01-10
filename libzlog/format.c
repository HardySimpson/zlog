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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "format.h"
#include "buf.h"
#include "spec.h"
#include "zc_defs.h"

/*******************************************************************************/
static void zlog_format_debug(zlog_format_t * a_format);
/*******************************************************************************/
zlog_format_t *zlog_format_new(const char *line, long line_len)
{
	int rc = 0;
	int nscan = 0;
	zlog_format_t *a_format = NULL;
	int nread;
	const char *p_start;
	const char *p_end;
	char *p;
	char *q;
	zlog_spec_t *a_spec;

	zc_assert_debug(line, NULL);

	a_format = calloc(1, sizeof(zlog_format_t));
	if (!a_format) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	/* line         &default                "$d(%F %X.%l) $-6P ($c:$F:$L) - $m$n"
	 * name         default
	 * pattern      $d(%F %X.%l) $-6P ($c:$F:$L) - $m$n
	 */

	memset(a_format->name, 0x00, sizeof(a_format->name));
	nscan = sscanf(line, "&%s %n", a_format->name, &nread);
	if (nscan != 1) {
		zc_error("sscanf [%s] error", line);
		rc = -1;
		goto zlog_format_new_exit;
	}

	for (p = a_format->name; *p != '\0'; p++) {
		if ((!isalnum(*p)) && (*p != '_')) {
			zc_error("a_format->name[%s] is not alpha, digit or _",
				 a_format->name);
			rc = -1;
			goto zlog_format_new_exit;
		}
	}

	if (*(line + nread) != '"') {
		zc_error("the 1st char of pattern is not \", line[%s]", line);
		rc = -1;
		goto zlog_format_new_exit;
	}

	p_start = line + nread + 1;
	p_end = strrchr(p_start, '"');
	if (!p_end) {
		zc_error("there is no \" at end of pattern, line[%s]", line);
		rc = -1;
		goto zlog_format_new_exit;
	}

	memset(a_format->pattern, 0x00, sizeof(a_format->pattern));
	strncpy(a_format->pattern, p_start, p_end - p_start);

	a_format->pattern_specs =
	    zc_arraylist_new((zc_arraylist_del_fn) zlog_spec_del);
	if (!(a_format->pattern_specs)) {
		zc_error("zc_arraylist_new fail");
		rc = -1;
		goto zlog_format_new_exit;
	}

	for (p = a_format->pattern; *p != '\0'; p = q) {
		a_spec = zlog_spec_new(p, &q);
		if (!a_spec) {
			zc_error("zlog_spec_new fail");
			rc = -1;
			goto zlog_format_new_exit;
		}

		rc = zc_arraylist_add(a_format->pattern_specs, a_spec);
		if (rc) {
			zc_error("zc_arraylist_add fail");
			rc = -1;
			goto zlog_format_new_exit;
		}
	}

      zlog_format_new_exit:
	if (rc) {
		zlog_format_del(a_format);
		return NULL;
	} else {
		zlog_format_debug(a_format);
		return a_format;
	}
}

void zlog_format_del(zlog_format_t * a_format)
{
	zc_assert_debug(a_format,);

	if (a_format->pattern_specs) {
		zc_arraylist_del(a_format->pattern_specs);
	}
	zc_debug("free a_format at [%p]", a_format);
	free(a_format);
	return;
}

/*******************************************************************************/
/* return 0	success
 * return !0	fail
 */
int zlog_format_gen_msg(zlog_format_t * a_format, zlog_thread_t * a_thread)
{
	int rc = 0;
	int i;
	zlog_spec_t *a_spec;

	zc_assert_debug(a_format, -1);
	zc_assert_debug(a_thread, -1);

	zlog_buf_restart(a_thread->msg_buf);

	zc_arraylist_foreach(a_format->pattern_specs, i, a_spec) {
		rc = zlog_spec_gen_msg(a_spec, a_thread);
		if (rc < 0) {
			zc_error("zlog_spec_gen_msg fail");
			return -1;
		} else if (rc > 0) {
			/* buf is full  */
			break;
		}
	}

	return 0;
}

/*******************************************************************************/
static void zlog_format_debug(zlog_format_t * a_format)
{
	zc_debug("format:[%p][%s]-[%s]", a_format,
		 a_format->name, a_format->pattern);
	return;
}

void zlog_format_profile(zlog_format_t * a_format)
{
	zc_assert_debug(a_format,);
	zc_error("format:[%p][%s]-[%s]", a_format,
		 a_format->name, a_format->pattern);
	return;
}

/*******************************************************************************/
