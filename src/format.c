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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "zc_defs.h"

     #include "event.h"
     #include "mdc.h"
  #include "thread.h"
#include "format.h"

#include "spec.h"

void zlog_format_profile(zlog_format_t * a_format, int flag)
{

	zc_assert(a_format,);
	zc_profile(flag, "---format[%p][%s = %s(%p)]---",
		a_format,
		a_format->name,
		a_format->pattern,
		a_format->pattern_specs);

#if 0
	int i;
	zlog_spec_t *a_spec;
	zc_arraylist_foreach(a_format->pattern_specs, i, a_spec) {
		zlog_spec_profile(a_spec, flag);
	}
#endif

	return;
}

/*******************************************************************************/
void zlog_format_del(zlog_format_t * a_format)
{
	zc_assert(a_format,);
	if (a_format->name) zc_sdsfree(a_format->name);
	if (a_format->pattern) zc_sdsfree(a_format->pattern);
	if (a_format->pattern_specs) zc_arraylist_del(a_format->pattern_specs);
	free(a_format);
	zc_debug("zlog_format_del[%p]", a_format);
	return;
}

zlog_format_t *zlog_format_new(char *line, int *time_cache_count)
{
	zlog_format_t *a_format = NULL;
	zc_sds *argv = NULL;
	int argc;
	int rc;
	char *p, *q;
	zlog_spec_t *a_spec;

	zc_assert(line, NULL);
	a_format = calloc(1, sizeof(zlog_format_t));
	if (!a_format) { zc_error("calloc fail, errno[%d]", errno); return NULL; }

	/* line         default = "%d(%F %X.%l) %-6V (%c:%F:%L) - %m%n"
	 * name         default
	 * pattern      %d(%F %X.%l) %-6V (%c:%F:%L) - %m%n
	 */
	argv = zc_sdssplitargs(line, "=", &argc);
	if (!argv) { zc_error("Unbalanced quotes in configuration line"); goto err; }
	if (argc != 2) { zc_error("level has 2 arguments, [%d]", argc); goto err; }

	a_format->pattern = zc_sdsreplaceenv(a_format->pattern);
	if (a_format->pattern) { zc_error("zc_str_replace_env fail"); goto err; }

	a_format->pattern_specs = zc_arraylist_new((zc_arraylist_del_fn) zlog_spec_del);
	if (!(a_format->pattern_specs)) { zc_error("zc_arraylist_new fail"); goto err; }

	for (p = a_format->pattern; *p != '\0'; p = q) {
		a_spec = zlog_spec_new(p, &q, time_cache_count);
		if (!a_spec) { zc_error("zlog_spec_new fail"); goto err; }

		rc = zc_arraylist_add(a_format->pattern_specs, a_spec)
		if (rc) {
			zlog_spec_del(a_spec);
			zc_error("zc_arraylist_add fail");
			goto err;
		}
	}

	if (argv) zc_sdsfreesplitres(argv, argc);
	zlog_format_profile(a_format, ZC_DEBUG);
	return a_format;
err:
	if (argv) zc_sdsfreesplitres(argv, argc);
	zlog_format_del(a_format);
	return NULL;
}

/*******************************************************************************/
/* return 0	success, or buf is full
 * return -1	fail
 */
int zlog_format_gen_msg(zlog_format_t * a_format, zlog_thread_t * a_thread)
{
	int i, rc;
	zlog_spec_t *a_spec;

	zlog_buf_restart(a_thread->msg_buf);

	zc_arraylist_foreach(a_format->pattern_specs, i, a_spec) {
		rc = zlog_spec_gen_msg(a_spec, a_thread);
		if (rc == 0) {
			return -1;
		}
	}

	return 0;
}
