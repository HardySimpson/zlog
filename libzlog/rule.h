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
#include "rotater.h"

typedef struct zlog_rule_s zlog_rule_t;

zlog_rule_t *zlog_rule_new(char *line,
		zlog_rotater_t * a_rotater,
		zc_arraylist_t * levels,
		zlog_format_t * default_format,
		zc_arraylist_t * formats,
		unsigned int file_perms);

void zlog_rule_del(zlog_rule_t * a_rule);
void zlog_rule_profile(zlog_rule_t * a_rule, int flag);

int zlog_rule_output(zlog_rule_t * a_rule, zlog_thread_t * a_thread);
int zlog_rule_match_category(zlog_rule_t * a_rule, char *category);
int zlog_rule_is_wastebin(zlog_rule_t * a_rule);

typedef int (*zlog_record_fn)(char *str2, char *msg, size_t msg_len);
int zlog_rule_set_record(zlog_rule_t * a_rule, char *str1, zlog_record_fn record);

#endif
