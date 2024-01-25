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

#ifndef __zlog_level_list_h
#define __zlog_level_list_h

#include "zc_defs.h"
#include "level.h"

zc_arraylist_t *zlog_level_list_new(void);
void zlog_level_list_del(zc_arraylist_t *levels);
void zlog_level_list_profile(zc_arraylist_t *levels, int flag);

/* conf init use, slow */
/* if l is wrong or str=="", return -1 */
int zlog_level_list_set(zc_arraylist_t *levels, char *line);

/* spec output use, fast */
/* rule output use, fast */
/* if not found, return levels[254] */
zlog_level_t *zlog_level_list_get(zc_arraylist_t *levels, int l);

/* rule init use, slow */
/* if not found, return -1 */
int zlog_level_list_atoi(zc_arraylist_t *levels, char *str);


#endif
