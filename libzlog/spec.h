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

#ifndef __zlog_spec_h
#define __zlog_spec_h

#include "event.h"
#include "buf.h"
#include "thread.h"

typedef struct zlog_spec_t zlog_spec_t;

typedef int (*zlog_spec_gen_buf_fn) (zlog_spec_t * a_spec,
		zlog_thread_t * a_thread, zlog_buf_t * a_buf);

typedef int (*zlog_spec_cook_thread_fn) (zlog_spec_t * a_spec, zlog_thread_t *a_thread);

struct zlog_spec_t {
	char *str;
	int len;

	char time_fmt[MAXLEN_CFG_LINE + 1];

	int ms_count;
	size_t ms_offset[MAXLEN_CFG_LINE / 2 + 2];
	int us_count;
	size_t us_offset[MAXLEN_CFG_LINE / 2 + 2];
	size_t time_len;

	char mdc_key[MAXLEN_PATH + 1];

	char print_fmt[MAXLEN_CFG_LINE + 1];
	zlog_spec_gen_buf_fn gen_buf;
	zlog_spec_cook_thread_fn gen_msg;
	zlog_spec_cook_thread_fn gen_path;
};

int zlog_spec_gen_msg(zlog_spec_t * a_spec, zlog_thread_t *a_thread);
int zlog_spec_gen_path(zlog_spec_t * a_spec, zlog_thread_t *a_thread);

zlog_spec_t * zlog_spec_new(char *pattern_start, char **pattern_end);
void zlog_spec_del(zlog_spec_t * a_spec);

void zlog_spec_profile(zlog_spec_t * a_spec);

#endif
