/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_thread_h
#define  __zlog_thread_h

typedef struct {
	int version;
	zlog_conf_t *conf;
	zlog_mdc_t *mdc;
	zlog_event_t *event;
	zc_hashtable_t *categories;
} zlog_thread_t;


zlog_thread_t *zlog_thread_new(int init_version, zlog_conf_t *a_conf);
void zlog_thread_del(zlog_thread_t * a_thread);
void zlog_thread_profile(zlog_thread_t * a_thread, int flag);

int zlog_thread_reload(zlog_conf_t * a_conf);

#endif
