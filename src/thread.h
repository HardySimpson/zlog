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

#ifndef __zlog_thread_h
#define  __zlog_thread_h

#include <stdatomic.h>
#include <stdbool.h>

#include "zc_defs.h"
#include "event.h"
#include "buf.h"
#include "mdc.h"

/**
 * zlog_thread_t -
 *
 * @producer: used with writer thread
 */
typedef struct zlog_thread_s {
	int init_version;
	zlog_mdc_t *mdc;
	zlog_event_t *event;

    /* change per conf start */
	zlog_buf_t *pre_path_buf;
	zlog_buf_t *path_buf;
	zlog_buf_t *archive_path_buf;
	zlog_buf_t *pre_msg_buf;
	zlog_buf_t *msg_buf;
    /* change per conf end */

    struct {
        /* change per conf start */
        bool en;
        atomic_int refcnt;
        /* change per conf end */
        unsigned int full_cnt;
    } producer;
} zlog_thread_t;

struct zlog_conf_s;
struct zlog_process_data;

void zlog_thread_del(zlog_thread_t * a_thread);
void zlog_thread_profile(zlog_thread_t * a_thread, int flag);
zlog_thread_t *zlog_thread_new(int init_version,
			size_t buf_size_min, size_t buf_size_max, int time_cache_count, struct zlog_conf_s *conf);

int zlog_thread_rebuild_msg_buf(zlog_thread_t * a_thread, size_t buf_size_min, size_t buf_size_max);
int zlog_thread_rebuild_event(zlog_thread_t * a_thread, int time_cache_count);

void zlog_thread_rebuild_producer(zlog_thread_t * thread, bool en);

#endif
