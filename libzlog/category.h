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

#ifndef __zlog_category_h
#define __zlog_category_h

#include "zc_defs.h"

typedef struct {
	zc_hashtable_t *tab;
} zlog_cmap_t;

int zlog_cmap_init(zlog_cmap_t * a_cmap);
int zlog_cmap_update(zlog_cmap_t * a_cmap, zc_arraylist_t * rules);
void zlog_cmap_fini(zlog_cmap_t * a_cmap);
void zlog_cmap_profile(zlog_cmap_t * a_cmap);

typedef struct zlog_category_t {
	char name[MAXLEN_PATH + 1];
	size_t name_len;
	zc_arraylist_t *match_rules;
} zlog_category_t;

zlog_category_t *zlog_cmap_fetch_category(zlog_cmap_t * a_cmap,
					  char *category_name,
					  zc_arraylist_t * rules);

#include "thread.h"
int zlog_category_output(zlog_category_t * a_cat, zlog_thread_t * a_thread);

#endif
