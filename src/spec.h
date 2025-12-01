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

#ifndef __zlog_spec_h
#define __zlog_spec_h

#include "event.h"
#include "buf.h"
#include "thread.h"

typedef struct zlog_spec_s zlog_spec_t;
struct zlog_output_data;

/* write buf, according to each spec's Conversion Characters */
typedef int (*zlog_spec_write_fn) (zlog_spec_t * a_spec,
			 	zlog_thread_t * a_thread,
			 	zlog_buf_t * a_buf, struct zlog_output_data *);

/* gen a_thread->msg or gen a_thread->path by using write_fn */
typedef int (*zlog_spec_gen_fn) (zlog_spec_t * a_spec,
				zlog_thread_t * a_thread, struct zlog_output_data *);


struct zlog_spec_s {
	char *str;
	int len;

	char time_fmt[MAXLEN_CFG_LINE + 1];
	int time_cache_index;
	char mdc_key[MAXLEN_PATH + 1];

	char print_fmt[MAXLEN_CFG_LINE + 1];
	int left_adjust;
	int left_fill_zeros;
	size_t max_width;
	size_t min_width;

	zlog_spec_write_fn write_buf;
	zlog_spec_gen_fn gen_msg;
	zlog_spec_gen_fn gen_path;
	zlog_spec_gen_fn gen_archive_path;
};

zlog_spec_t *zlog_spec_new(char *pattern_start, char **pattern_end, int * time_cache_count);
void zlog_spec_del(zlog_spec_t * a_spec);
void zlog_spec_profile(zlog_spec_t * a_spec, int flag);

#define zlog_spec_gen_msg(a_spec, a_thread, data) \
	a_spec->gen_msg(a_spec, a_thread, data)

#define zlog_spec_gen_path(a_spec, a_thread, data) \
	a_spec->gen_path(a_spec, a_thread, data)

#define zlog_spec_gen_archive_path(a_spec, a_thread) \
	a_spec->gen_archive_path(a_spec, a_thread, NULL)

#endif
