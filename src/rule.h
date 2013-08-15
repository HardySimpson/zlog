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

typedef int (*zlog_rule_output_fn) (zlog_rule_t * a_rule, zlog_event_t * a_event, zlog_mdc_t *a_mdc);
typedef int (*zlog_rule_flush_fn) (zlog_rule_t * a_rule);

struct zlog_rule_s {
	zc_sds cname;
	unsigned char level_bitmap[32];

	zc_sds buffer;
	size_t flush_count;
	size_t fsync_count;

	zlog_deepness_t *file_deep;
	zc_sds file_path;
	zc_sds file_path_dynamic;
	zc_arraylist_t *file_path_specs;
	int file_fd;

	size_t archive_max_size;
	int archive_max_count;
	zc_sds archive_path;
	zc_arraylist_t *archive_specs;

	FILE *pipe_fp;
	int pipe_fd;

	int syslog_facility;

	zc_arraylist_t *levels;
	zlog_format_t *format;

	zlog_rule_output_fn output;
	zlog_rule_flush_fn flush;
	zlog_record_fn record_func;
};

zlog_rule_t *zlog_rule_new(char * line, zlog_conf_t *a_conf);
void zlog_rule_del(zlog_rule_t * a_rule);
void zlog_rule_profile(zlog_rule_t * a_rule, int flag);
zlog_rule_t *zlog_rule_dup(zlog_rule_t * a_rule);

int zlog_rule_match_cname(zlog_rule_t * a_rule, char *cname);
int zlog_rule_set_record(zlog_rule_t * a_rule, zc_hashtable_t *records);

int zlog_rule_flush(zlog_rule_t * a_rule);

#define zlog_rule_output(a_rule, event, mdc) a_rule->output(a_rule, event, mdc)
#define zlog_rule_flush(a_rule) a_rule->flush(a_rule)

#define zlog_rule_has_level(a_rule, lv)   \
		(a_rule->level_bitmap[lv/8] & (0x1 << (i % 8))

#define zlog_rule_is_wastebin(a_rule) (a_rule && STRCMP(a_rule->category, ==, "!"))

#endif
