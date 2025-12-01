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

#ifndef __zlog_format_h
#define __zlog_format_h

#include "thread.h"
#include "zc_defs.h"

typedef struct zlog_format_s zlog_format_t;

struct zlog_format_s {
	char name[MAXLEN_CFG_LINE + 1];	
	char pattern[MAXLEN_CFG_LINE + 1];
	zc_arraylist_t *pattern_specs;
};

zlog_format_t *zlog_format_new(char *line, int * time_cache_count);
void zlog_format_del(zlog_format_t * a_format);
void zlog_format_profile(zlog_format_t * a_format, int flag);

struct zlog_output_data;
int zlog_format_gen_msg(zlog_format_t * a_format, zlog_thread_t * a_thread, struct zlog_output_data *data);

#define zlog_format_has_name(a_format, fname) \
	STRCMP(a_format->name, ==, fname)

#endif
