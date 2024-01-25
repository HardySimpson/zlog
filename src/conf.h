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

#ifndef __zlog_conf_h
#define __zlog_conf_h

#include "zc_defs.h"
#include "format.h"
#include "rotater.h"

typedef struct zlog_conf_s {
	char file[MAXLEN_PATH + 1];
	char cfg_ptr[MAXLEN_CFG_LINE*MAXLINES_NO];
	char mtime[20 + 1];

	int strict_init;
	size_t buf_size_min;
	size_t buf_size_max;

	char rotate_lock_file[MAXLEN_CFG_LINE + 1];
	zlog_rotater_t *rotater;

	char default_format_line[MAXLEN_CFG_LINE + 1];
	zlog_format_t *default_format;

	unsigned int file_perms;
	size_t fsync_period;
	size_t reload_conf_period;

	zc_arraylist_t *levels;
	zc_arraylist_t *formats;
	zc_arraylist_t *rules;
	int time_cache_count;
	char log_level[MAXLEN_CFG_LINE + 1];
	int level;
} zlog_conf_t;

extern zlog_conf_t * zlog_env_conf;

zlog_conf_t *zlog_conf_new(const char *config);
zlog_conf_t *zlog_conf_new_from_string(const char *config_string);
void zlog_conf_del(zlog_conf_t * a_conf);
void zlog_conf_profile(zlog_conf_t * a_conf, int flag);

#endif
