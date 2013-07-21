/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_conf_h
#define __zlog_conf_h

typedef struct zlog_conf_s {
	zc_sds file;
	long mtime;

	int strict_init;
	long expire_time;

	zc_sds rotate_lock_file;
	zc_sds default_deepness_str;
	zc_sds default_format_str;

	zlog_deepness_t *default_deepness;
	zlog_format_t *default_format;

	zc_arraylist_t *deepness;
	zc_arraylist_t *levels;
	zc_arraylist_t *formats;
	zc_arraylist_t *rules;
} zlog_conf_t;

zlog_conf_t *zlog_conf_new(const char *confpath);
void zlog_conf_del(zlog_conf_t * a_conf);
void zlog_conf_profile(zlog_conf_t * a_conf, int flag);

zlog_conf_t *zlog_conf_dup(zlog_conf_t *a_conf);

#endif
