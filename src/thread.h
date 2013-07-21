/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_thread_h
#define  __zlog_thread_h

typedef zlog_category_s zlog_category_t;

typedef struct zlog_thread_s {
	int version;		/* compare to zlog_env_version, for update conf */
	int idx;		/* index of zlog_env_threads, for cleanup */
	zlog_conf_t *conf;	/* deep copy of zlog_env_conf */
	zlog_mdc_t *mdc;	/* tag map */
	zlog_event_t *event;	/* info of each log action */
	zlog_category_t *default_category;	/* has the same category name in dzlog api */
	zc_hashtable_t *categories;	/* all exist categories of this thread */
} zlog_thread_t;


zlog_thread_t *zlog_thread_new(zlog_conf_t *a_conf);
void zlog_thread_del(zlog_thread_t * a_thread);
void zlog_thread_profile(zlog_thread_t * a_thread, int flag);

#define zlog_thread_set_default_category(t, c) (t->default_category = (c))

zlog_category_t *zlog_thread_fetch_category(zlog_thread_t * a_thread, const char *cname, zc_hashtable_t *records);
int zlog_thread_update(zlog_thread_t *a_thread, zlog_conf_t * a_conf, zc_hashtable_t *records, int version);

#endif
