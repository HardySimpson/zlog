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

#ifndef __zlog_category_h
#define __zlog_category_h

#include "zc_defs.h"
#include "thread.h"

typedef struct zlog_category_s {
	char name[MAXLEN_PATH + 1];
	size_t name_len;
	unsigned char level_bitmap[32];
	unsigned char level_bitmap_backup[32];
	zc_arraylist_t *fit_rules;
	zc_arraylist_t *fit_rules_backup;
} zlog_category_t;

zlog_category_t *zlog_category_new(const char *name, zc_arraylist_t * rules);
void zlog_category_del(zlog_category_t * a_category);
void zlog_category_profile(zlog_category_t *a_category, int flag);

int zlog_category_update_rules(zlog_category_t * a_category, zc_arraylist_t * new_rules);
void zlog_category_commit_rules(zlog_category_t * a_category);
void zlog_category_rollback_rules(zlog_category_t * a_category);

struct zlog_output_data;
int zlog_category_output(zlog_category_t * a_category, zlog_thread_t * a_thread, struct zlog_output_data *data);

#define zlog_category_needless_level(a_category, lv) \
        a_category && (zlog_env_conf->level > lv || !((a_category->level_bitmap[lv/8] >> (7 - lv % 8)) & 0x01))

#endif
