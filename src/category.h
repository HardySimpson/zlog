/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

/* 
 * category is
 * 1. the union of all fit rules for output
 * 2. live only in one thread,
      and has the rights to access all info of the thread,
      category -> fit rules -> formats -> specs
         |-> mdc     |-> levels <-|
	 |-> event

 * rules may depend other 2 things for output
 * 1. zlog_env_rotater, access by lock
 * 2. one record of rezlog_env_records,
      as record is a funciton pointer stays in a static place, 
      just keep its adress is safe
 */

#ifndef __zlog_category_h
#define __zlog_category_h


typedef struct zlog_category_s {
	zc_sds name;
	unsigned char level_bitmap[32];
	zc_arraylist_t *fit_rules;

	/* link from thread */
	int version;
	zlog_event_t *event;
	zlog_mdc_t *mdc;
} zlog_category_t;

zlog_category_t *zlog_category_new(const char *name, zc_arraylist *rules,
				int version, zlog_event_t *event, zlog_mdc_t *mdc);

void zlog_category_del(zlog_category_t * a_category);
void zlog_category_profile(zlog_category_t *a_category, int flag);

int zlog_category_output(zlog_category_t * a_category);
int zlog_category_flush(zlog_category_t * a_category);

#define zlog_category_without_level(a_category, lv) \
        !(a_category->level_bitmap[lv/8] & (0x1 << (i % 8)))

zc_hashtable_type_t zlog_category_hash_type = {
	zc_hashtable_str_hash;
	zc_hashtable_str_equal;
	NULL;
	zc_category_del;
	NULL;
	NULL;
};

#endif
