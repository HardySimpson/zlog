/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_category_h
#define __zlog_category_h

typedef struct zlog_category_s {
	zc_sds cname;
	unsigned char level_bitmap[32];
	zc_arraylist_t *rules;
	zlog_mdc_t *mdc;
	zlog_event_t *event;
} zlog_category_t;

zlog_category_t *zlog_category_new(const char *name);
int zlog_category_add_rule(zlog_category_t * a_category, zlog_rule_t *a_rule);
void zlog_category_del(zlog_category_t * a_category);
void zlog_category_profile(zlog_category_t *a_category, int flag);

int zlog_category_output(zlog_category_t * a_category);
int zlog_category_flush(zlog_category_t * a_category);

#define zlog_category_without_level(a_category, lv) \
        !(a_category->level_bitmap[lv/8] & (0x1 << (lv % 8)))

zc_hashtable_type_t zlog_category_hash_type = {
	zc_hashtable_str_hash;
	zc_hashtable_str_equal;
	NULL;
	zc_category_del;
	NULL;
	NULL;
};

#endif
