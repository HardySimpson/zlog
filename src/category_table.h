/* Copyright (c) Hardy Simpson
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __zlog_category_table_h
#define __zlog_category_table_h

#include "zc_defs.h"
#include "category.h"

zc_hashtable_t *zlog_category_table_new(void);
void zlog_category_table_del(zc_hashtable_t * categories);
void zlog_category_table_profile(zc_hashtable_t * categories, int flag);

/* if none, create new and return */
zlog_category_t *zlog_category_table_fetch_category(
			zc_hashtable_t * categories,
		 	const char *category_name, zc_arraylist_t * rules);

int zlog_category_table_update_rules(zc_hashtable_t * categories, zc_arraylist_t * new_rules);
void zlog_category_table_commit_rules(zc_hashtable_t * categories);
void zlog_category_table_rollback_rules(zc_hashtable_t * categories);

#endif
