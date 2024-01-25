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

#ifndef __zlog_level_h
#define __zlog_level_h

#include "zc_defs.h"

typedef struct zlog_level_s {
	int int_level;
	char str_uppercase[MAXLEN_PATH + 1];
	char str_lowercase[MAXLEN_PATH + 1];
	size_t str_len;
       	int syslog_level;
} zlog_level_t;

zlog_level_t *zlog_level_new(char *line);
void zlog_level_del(zlog_level_t *a_level);
void zlog_level_profile(zlog_level_t *a_level, int flag);

#endif
