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

#ifndef __zlog_mdc_h
#define __zlog_mdc_h

#include "zc_defs.h"

typedef struct zlog_mdc_s zlog_mdc_t;
struct zlog_mdc_s {
	zc_hashtable_t *tab;
};

zlog_mdc_t *zlog_mdc_new(void);
void zlog_mdc_del(zlog_mdc_t * a_mdc);
void zlog_mdc_profile(zlog_mdc_t *a_mdc, int flag);

void zlog_mdc_clean(zlog_mdc_t * a_mdc);
int zlog_mdc_put(zlog_mdc_t * a_mdc, const char *key, const char *value);
char *zlog_mdc_get(zlog_mdc_t * a_mdc, const char *key);
void zlog_mdc_remove(zlog_mdc_t * a_mdc, const char *key);

typedef struct zlog_mdc_kv_s {
	char key[MAXLEN_PATH + 1];
	char value[MAXLEN_PATH + 1];
	size_t value_len;
} zlog_mdc_kv_t;

zlog_mdc_kv_t *zlog_mdc_get_kv(zlog_mdc_t * a_mdc, const char *key);

#endif
