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

#ifndef __zlog_record_h
#define __zlog_record_h

#include "zc_defs.h"

/* record is user-defined output function and it's name from configure file */
typedef struct zlog_msg_s {
	char *buf;
	size_t len;
	char *path;
} zlog_msg_t; /* 3 of this first, see need thread or not later */

typedef int (*zlog_record_fn)(zlog_msg_t * msg);

typedef struct zlog_record_s {
	char name[MAXLEN_PATH + 1];
	zlog_record_fn output;
} zlog_record_t;

zlog_record_t *zlog_record_new(const char *name, zlog_record_fn output);
void zlog_record_del(zlog_record_t *a_record);
void zlog_record_profile(zlog_record_t *a_record, int flag);

#endif
