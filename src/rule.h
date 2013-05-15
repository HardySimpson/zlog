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

typedef struct zlog_rule_s zlog_rule_t;

typedef int (*zlog_rule_output_fn) (zlog_rule_t * a_rule, zlog_thread_t * a_thread);

struct zlog_rule_s {
	zc_sds category;
	char compare_char;
	/* 
	 * [*] log all level
	 * [.] log level >= rule level, default
	 * [=] log level == rule level 
	 * [!] log level != rule level
	 */
	int level;
	unsigned char level_bitmap[32]; /* for category determine whether ouput or not */

	unsigned int file_perms;
	int file_open_flags;

	zc_sds file_path;
	zc_arraylist_t *dynamic_specs;
	int static_fd;

	long archive_max_size;
	int archive_max_count;
	//char archive_path[MAXLEN_PATH + 1];
	zc_arraylist_t *archive_specs;

	FILE *pipe_fp;
	int pipe_fd;

	size_t fsync_period;
	size_t fsync_count;

	zc_arraylist_t *levels;
	int syslog_facility;

	zlog_format_t *format;
	zlog_rule_output_fn output;

	//char record_name[MAXLEN_PATH + 1];
	//char record_path[MAXLEN_PATH + 1];
	//zlog_record_fn record_func;
};

zlog_rule_t *zlog_rule_new(char * line, zlog_conf_t * a_conf);
void zlog_rule_del(zlog_rule_t * a_rule);
void zlog_rule_profile(zlog_rule_t * a_rule, int flag);
int zlog_rule_match_category(zlog_rule_t * a_rule, char *category);
int zlog_rule_set_record(zlog_rule_t * a_rule, zc_hashtable_t *records);
int zlog_rule_output(zlog_rule_t * a_rule, zlog_thread_t * a_thread);

#define zlog_rule_is_wastebin(a_rule) (a_rule && STRCMP(a_rule->category, ==, "!"))

#endif
