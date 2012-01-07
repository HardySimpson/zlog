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

/**
 * @file category.h
 * @brief zlog category and cmap(global hashtable of categories)
 */

/**
 * Cats is the hashtable of category
 */
typedef struct {
	zc_hashtable_t *tab;	/**< the table */
} zlog_cmap_t;

/**
 * Cats initer
 *
 * zlog_cmap_init() will initialise hashtable.
 * @param a_cmap the address pointed to by zlog_cmap_t.
 * @return 0 for success, -1 for fail
 */
int zlog_cmap_init(zlog_cmap_t * a_cmap);

/**
 * Update all categories in a_cmap
 *
 * zlog_cmap_update() make all categories update their rules from rule list.
 * @param a_cmap the address pointed to by zlog_cmap_t.
 * @param rules list of all rules
 * @return 0 for success, -1 for fail
 */
int zlog_cmap_update(zlog_cmap_t *a_cmap, zc_arraylist_t *rules);

/**
 * Cats finisher
 *
 * zlog_cmap_fini() will del all categories and destroy hashtable.
 * @param a_cmap the address pointed to by zlog_cmap_t.
 */
void zlog_cmap_fini(zlog_cmap_t * a_cmap);

/**
 * Output detail of zlog_cmap_t to ZLOG_ERROR_LOG.
 *
 * @param a_cmap the address pointed to by zlog_cmap_t.
 */
void zlog_cmap_profile(zlog_cmap_t * a_cmap);

/**
 * zlog category struct
 */
typedef struct zlog_category_t {
	char name[MAXLEN_PATH + 1];	/**< name, must consist of alpha, digits or _ */
	size_t name_len;		/**< strlen(name) */
	zc_arraylist_t *match_rules;	/**< list of rules which match this category */
} zlog_category_t;

/**
 * Get a zlog_category_t from hashtable, if not found, use rules to create one.
 *
 * @param a_cmap the address pointed to by zlog_cmap_t.
 * @param category_name name of category
 * @param rules if not found, use rules to create a new category
 * @returns poiter of zlog_category_t for success, NULL for fail
 */
zlog_category_t *zlog_cmap_fetch_category(zlog_cmap_t *a_cmap,
				char *category_name, zc_arraylist_t *rules);

#include "thread.h"

/**
 * Output one message to several rules.
 *
 * @param a_cat contains all rules that maybe use to output, up to priority
 * @param a_thread the message's raw material and formated buffer are in a_thread,
 * which belongs to one thread
 * @returns 0 for success, -1 for fail
 */
int zlog_category_output(zlog_category_t *a_cat, zlog_thread_t *a_thread);

#endif
