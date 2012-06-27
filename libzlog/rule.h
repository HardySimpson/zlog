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
#include <signal.h>
#include "zc_defs.h"
#include "format.h"
#include "thread.h"
#include "rotater.h"
#include "record.h"

typedef struct zlog_rule_s zlog_rule_t;

typedef int (*zlog_rule_output_fn) (zlog_rule_t * a_rule, zlog_thread_t * a_thread);

struct zlog_rule_s {
	char category[MAXLEN_CFG_LINE + 1];
	int level;
	char compare_char;
	/* 
	 * [*] log all level
	 * [.] log level >= rule level, default
	 * [=] log level == rule level 
	 * [!] log level != rule level
	 */

	char file_path[MAXLEN_PATH + 1];
	zc_arraylist_t *dynamic_file_specs;
	int static_file_descriptor;
	FILE *static_file_stream;
	pthread_rwlock_t static_reopen_lock;

	unsigned int file_perms;
	int file_open_flags;
	long file_max_size;
	int file_max_count;

	size_t fsync_period;
	volatile sig_atomic_t fsync_count;

	zc_arraylist_t *levels;
	int syslog_facility;

	zlog_format_t *format;
	zlog_rotater_t *rotater;

	zlog_rule_output_fn output;

	char record_name[MAXLEN_PATH + 1];
	char record_param[MAXLEN_PATH + 1];
	zlog_record_fn record_output;
};

zlog_rule_t *zlog_rule_new(char *line,
		zlog_rotater_t * a_rotater,
		zc_arraylist_t * levels,
		zlog_format_t * default_format,
		zc_arraylist_t * formats,
		unsigned int file_perms,
		size_t fsync_period);

void zlog_rule_del(zlog_rule_t * a_rule);
void zlog_rule_profile(zlog_rule_t * a_rule, int flag);

int zlog_rule_match_category(zlog_rule_t * a_rule, char *category);
int zlog_rule_is_wastebin(zlog_rule_t * a_rule);

int zlog_rule_set_record(zlog_rule_t * a_rule, zc_hashtable_t *records);

int zlog_rule_output(zlog_rule_t * a_rule, zlog_thread_t * a_thread);
#if 0
#define zlog_rule_output(a_rule, a_thread) do { \
	switch (a_rule->compare_char) {   \
	case '*' :   \
		a_rule->output(a_rule, a_thread);   \
		break;   \
	case '.' :   \
		if (a_thread->event->level >= a_rule->level) {   \
			a_rule->output(a_rule, a_thread);   \
		}   \
		break;   \
	case '=' :   \
		if (a_thread->event->level == a_rule->level) {   \
			a_rule->output(a_rule, a_thread);   \
		}   \
		break;   \
	case '!' :   \
		if (a_thread->event->level != a_rule->level) {   \
			a_rule->output(a_rule, a_thread);   \
		}   \
		break;   \
	}   \
} while(0) 
#endif

#define zlog_rule_should_output(a_rule, l, result) do { \
	switch (a_rule->compare_char) {    \
	case '*' :    \
		result = 1;    \
		break;    \
	case '.' :    \
		(l >= a_rule->level) ? (result = 1) : (result = 0);    \
		break;    \
	case '=' :    \
		(l == a_rule->level) ? (result = 1) : (result = 0);    \
		break;    \
	case '!' :    \
		(l != a_rule->level) ? (result = 1) : (result = 0);    \
		break;    \
	}    \
} while(0)


#endif
