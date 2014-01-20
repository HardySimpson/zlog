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
typedef int (*zlog_udf) (char *msg, size_t msg_len, char *path, size_t path_len);

struct zlog_rule_s {
	zc_sds cname;
	unsigned char level_bitmap[32];

	zc_sds output_type;

	zc_sds file_path;
	zc_arraylist_t *file_path_specs;

	int file_fd;
	unsigned int file_permisson;
	size_t buffer_len;
	size_t flush_len;
	size_t flush_count;
	ssize_t fsync_count;

	size_t archive_max_size;
	int archive_max_count;
	zc_sds archive_path;
	zc_arraylist_t *archive_path_specs;

	int syslog_facility;

	FILE *pipe_fp;
	int pipe_fd;

	zc_sds msg_buffer;
	zc_sds file_path_dynamic;

	zc_sds format_pattern;
	zc_arraylist_t *format_pattern_specs;

	zlog_rule_output_fn output;
	zlog_rule_flush_fn flush;

	zlog_udf udf;
};

zlog_rule_t *zlog_rule_new(char *cname, char compare, int level);

/*
 * key			value
 * output_type		[file,stdout,stderr,pipe,syslog,user_defile]

 * syslog_facility	[LOG_LOCAL0-7,LOG_USER...] see man 3 syslog

 * file_path		"/path/to/log/file"
 * file_permisson	"0644"
 * buffer_len		"1K"
 * flush_len		"800"
 * flush_count		"20"
 * fsync_count		"1000"
 * archive_max_size	"50M"
 * archive_max_count	"10"
 * archive_path		"/path/to/log/archive_file#2r"
 *
 * pipe_programm	"/usr/bin/cronolog /www/logs/example_%Y%m%d.log"
 *
 * format_pattern	"%d %m%n"
 */
int zlog_rule_set(zlog_rule_t *a_rule, char *key, void *value);
#define zlog_rule_set_udf(a_rule, a_udf) \
	a_rule->udf = a_udf;
void zlog_rule_del(zlog_rule_t * a_rule);
void zlog_rule_profile(zlog_rule_t * a_rule, int flag);
zlog_rule_t *zlog_rule_dup(zlog_rule_t *a_rule);

int zlog_rule_match_category_name(zlog_rule_t * a_rule, char *category_name);

#define zlog_rule_output(a_rule, a_event, a_mdc) a_rule->output(a_rule, a_event, a_mdc)
#define zlog_rule_flush(a_rule) a_rule->flush(a_rule)
#define zlog_rule_has_level(a_rule, lv)   \
		(a_rule->level_bitmap[lv/8] & (0x1 << (lv % 8))

#endif
