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

#ifndef __zlog_format_h
#define __zlog_format_h

#include "thread.h"
#include "zc_defs.h"

typedef struct zlog_format_s zlog_format_t;

struct zlog_format_s {
	char name[MAXLEN_CFG_LINE + 1];	
	char pattern[MAXLEN_CFG_LINE + 1];
	zc_arraylist_t *pattern_specs;
};

zlog_format_t *zlog_format_new(char *line);
void zlog_format_del(zlog_format_t * a_format);
void zlog_format_profile(zlog_format_t * a_format, int flag);

int zlog_format_gen_msg(zlog_format_t * a_format, zlog_thread_t * a_thread);

#define zlog_format_has_name(a_format, fname) \
	STRCMP(a_format->name, ==, fname)

#endif
