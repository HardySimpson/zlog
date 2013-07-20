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


void zlog_thread_del(zlog_thread_t * a_thread);
void zlog_thread_profile(zlog_thread_t * a_thread, int flag);
zlog_thread_t *zlog_thread_new(int init_version,
			size_t buf_size_min, size_t buf_size_max, int time_cache_count);

int zlog_thread_rebuild_msg_buf(zlog_thread_t * a_thread, size_t buf_size_min, size_t buf_size_max);
int zlog_thread_rebuild_event(zlog_thread_t * a_thread, int time_cache_count);

#endif
