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

#ifndef __zlog_record_table_h
#define __zlog_record_table_h

#include "zc_defs.h"
#include "record.h"

zc_hashtable_t *zlog_record_table_new(void);
void zlog_record_table_del(zc_hashtable_t * records);
void zlog_record_table_profile(zc_hashtable_t * records, int flag);

#endif
