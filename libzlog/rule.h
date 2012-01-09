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

#ifndef __zlog_rule_h
#define __zlog_rule_h

#include <stdio.h>
#include "zc_defs.h"
#include "format.h"
#include "thread.h"

typedef struct zlog_rule_t zlog_rule_t;
typedef int (*zlog_rule_output_fn) (zlog_rule_t * a_rule,
				    zlog_thread_t * a_thread);
struct zlog_rule_t {
	char category[MAXLEN_CFG_LINE + 1];
	char compare_char;
	/* [*] log any priority 
	 * [.] log priority >= rule priority, default
	 * [=] log priority == rule priority 
	 * [!] log priority != rule priority
	 */
	int priority;

	char file_path[MAXLEN_PATH + 1];
	FILE *static_file_stream;
	zc_arraylist_t *dynamic_file_specs;

	long file_maxsize;
	int syslog_facility;

	zlog_rule_output_fn output_fn;

	zlog_format_t *format;
};

zlog_rule_t *zlog_rule_new(zc_arraylist_t * formats, char *line, long line_len);
void zlog_rule_del(zlog_rule_t * a_rule);

int zlog_rule_output(zlog_rule_t * a_rule, zlog_thread_t * a_thread);

int zlog_rule_match_category(zlog_rule_t * a_rule, char *category);

void zlog_rule_profile(zlog_rule_t * a_rule);

#endif
