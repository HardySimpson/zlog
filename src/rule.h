/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */


#ifndef __zlog_rule_h
#define __zlog_rule_h

typedef struct zlog_rule_s zlog_rule_t;

typedef int (*zlog_rule_output_fn) (zlog_rule_t * a_rule, zlog_thread_t * a_thread);

struct zlog_rule_s {
	zc_sds cname;
	unsigned char level_charmap[256]; /* if (level_charmap[i] != 0) allow output */

	zlog_deepness_t *file_deep;
	zc_sds file_path;
	zc_arraylist_t *file_path_specs;
	int file_fd;

	size_t archive_max_size;
	int archive_max_count;
	zc_sds archive_path;
	zc_arraylist_t *archive_specs;

	FILE *pipe_fp;
	int pipe_fd;

	int syslog_facility;

	zlog_format_t *format;
	zlog_rule_output_fn output;

	//char record_name[MAXLEN_PATH + 1];
	//char record_path[MAXLEN_PATH + 1];
	zlog_record_fn record_func;
};

zlog_rule_t *zlog_rule_new(char * line, zlog_conf_t *a_conf);
void zlog_rule_del(zlog_rule_t * a_rule);
void zlog_rule_profile(zlog_rule_t * a_rule, int flag);
zlog_rule_t *zlog_rule_dup(zlog_rule_t * a_rule);


int zlog_rule_match_cname(zlog_rule_t * a_rule, char *cname);
int zlog_rule_set_record(zlog_rule_t * a_rule, zc_hashtable_t *records);
int zlog_rule_output(zlog_rule_t * a_rule, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds msg);

#define zlog_rule_output(a_rule, event, mdc, msg) a_rule->output(event, mdc, msg)
#define zlog_rule_has_level(a_rule, lv) (a_rule->level_charmap[lv])
#define zlog_rule_is_wastebin(a_rule) (a_rule && STRCMP(a_rule->category, ==, "!"))

#endif
