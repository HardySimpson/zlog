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

/**
 * @file rule.h
 * @brief rule decide to output in format by category & level
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
	/* [*] log any level 
	 * [.] log level >= rule level, default
	 * [=] log level == rule level 
	 * [!] log level != rule level
	 */
	int level;

	char file_path[MAXLEN_PATH + 1];
	FILE *static_file_stream;
	zc_arraylist_t *dynamic_file_specs;

	long file_maxsize;
	int syslog_facility;

	zlog_rule_output_fn output;

	zlog_format_t *format;
};

zlog_rule_t *zlog_rule_new(zlog_format_t *default_format,
		zc_arraylist_t * formats, char *line, long line_len);
void zlog_rule_del(zlog_rule_t * a_rule);

int zlog_rule_output(zlog_rule_t * a_rule, zlog_thread_t * a_thread);

int zlog_rule_match_category(zlog_rule_t * a_rule, char *category);
int zlog_rule_is_wastebin(zlog_rule_t * a_rule);

void zlog_rule_profile(zlog_rule_t * a_rule);

#endif
