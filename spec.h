/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __xlog_spec_h
#define __xlog_spec_h

#include "event.h"
#include "buf.h"
#include "thread.h"

typedef struct xlog_spec_t xlog_spec_t;

typedef int (*xlog_spec_gen_fn) (xlog_spec_t * a_spec,
		xlog_thread_t * a_thread, xlog_buf_t * a_buf);

struct xlog_spec_t {
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
	xlog_spec_gen_fn gen_fn;
};

int xlog_spec_gen_msg(xlog_spec_t * a_spec, xlog_thread_t *a_thread);
int xlog_spec_gen_path(xlog_spec_t * a_spec, xlog_thread_t *a_thread);

xlog_spec_t * xlog_spec_new(char *pattern_start, char **pattern_end);
void xlog_spec_del(xlog_spec_t * a_spec);

void xlog_spec_profile(xlog_spec_t * a_spec);

#endif
