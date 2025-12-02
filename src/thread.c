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

#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <stdatomic.h>

#include "buf.h"
#include "conf.h"
#include "event.h"
#include "mdc.h"
#include "misc.h"
#include "thread.h"
#include "zc_defs.h"

void zlog_thread_profile(zlog_thread_t * a_thread, int flag)
{
	zc_assert(a_thread,);
	zc_profile(flag, "--thread[%p][%p][%p][%p,%p,%p,%p,%p]--",
			a_thread,
			a_thread->mdc,
			a_thread->event,
			a_thread->pre_path_buf,
			a_thread->path_buf,
			a_thread->archive_path_buf,
			a_thread->pre_msg_buf,
			a_thread->msg_buf);

	zlog_mdc_profile(a_thread->mdc, flag);
	zlog_event_profile(a_thread->event, flag);
	zlog_buf_profile(a_thread->pre_path_buf, flag);
	zlog_buf_profile(a_thread->path_buf, flag);
	zlog_buf_profile(a_thread->archive_path_buf, flag);
    if (a_thread->pre_msg_buf) {
        zlog_buf_profile(a_thread->pre_msg_buf, flag);
    }
    if (a_thread->msg_buf) {
        zlog_buf_profile(a_thread->msg_buf, flag);
    }
    return;
}
/*******************************************************************************/
void zlog_thread_del(zlog_thread_t * a_thread)
{
	zc_assert(a_thread,);
	if (a_thread->producer.en) {
        if (atomic_fetch_sub(&a_thread->producer.refcnt, 1) > 1) {
            return;
        }
        zc_debug("fullcnt %d\n", a_thread->producer.full_cnt);
	}
    zc_debug("zlog_thread_del[%lx], producer en %d, cnt %d", a_thread->event->tid,
             a_thread->producer.en,
             atomic_load_explicit(&a_thread->producer.refcnt, memory_order_relaxed));
    if (a_thread->mdc)
        zlog_mdc_del(a_thread->mdc);
    if (a_thread->event)
        zlog_event_del(a_thread->event);
    if (a_thread->pre_path_buf)
        zlog_buf_del(a_thread->pre_path_buf);
    if (a_thread->path_buf)
        zlog_buf_del(a_thread->path_buf);
    if (a_thread->archive_path_buf)
        zlog_buf_del(a_thread->archive_path_buf);
    if (a_thread->pre_msg_buf)
        zlog_buf_del(a_thread->pre_msg_buf);
    if (a_thread->msg_buf)
        zlog_buf_del(a_thread->msg_buf);

    free(a_thread);
	return;
}

zlog_thread_t *zlog_thread_new(int init_version, size_t buf_size_min, size_t buf_size_max, int time_cache_count, zlog_conf_t *conf)
{
	zlog_thread_t *a_thread;

	a_thread = calloc(1, sizeof(zlog_thread_t));
	if (!a_thread) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_thread->init_version = init_version;

	a_thread->mdc = zlog_mdc_new();
	if (!a_thread->mdc) {
		zc_error("zlog_mdc_new fail");
		goto err;
	}

	a_thread->event = zlog_event_new(time_cache_count);
	if (!a_thread->event) {
		zc_error("zlog_event_new fail");
		goto err;
	}

	a_thread->pre_path_buf = zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->pre_path_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	a_thread->path_buf = zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->path_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

	a_thread->archive_path_buf = zlog_buf_new(MAXLEN_PATH + 1, MAXLEN_PATH + 1, NULL);
	if (!a_thread->archive_path_buf) {
		zc_error("zlog_buf_new fail");
		goto err;
	}

    if (!conf->log_consumer.en) {
        a_thread->pre_msg_buf = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
        if (!a_thread->pre_msg_buf) {
            zc_error("zlog_buf_new fail");
            goto err;
        }

        a_thread->msg_buf = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
        if (!a_thread->msg_buf) {
            zc_error("zlog_buf_new fail");
            goto err;
        }
    }

    if (conf->log_consumer.en) {
        a_thread->producer.en = true;
        atomic_init(&a_thread->producer.refcnt, 1);
        zc_debug("init %lx, refcnt %d", pthread_self(),
                 atomic_load_explicit(&a_thread->producer.refcnt, memory_order_relaxed));
    }

    // zlog_thread_profile(a_thread, ZC_DEBUG);
    return a_thread;
err:
	zlog_thread_del(a_thread);
	return NULL;
}

/*******************************************************************************/
int zlog_thread_rebuild_msg_buf(zlog_thread_t * a_thread, size_t buf_size_min, size_t buf_size_max)
{
	zlog_buf_t *pre_msg_buf_new = NULL;
	zlog_buf_t *msg_buf_new = NULL;
	zc_assert(a_thread, -1);

    if (a_thread->msg_buf && !a_thread->producer.en) {
        /* prev and cur disable producer */
        if ((a_thread->msg_buf->size_min == buf_size_min) &&
            (a_thread->msg_buf->size_max == buf_size_max)) {
            zc_debug("buf size not changed, no need rebuild");
            return 0;
        }
    }

    if (!a_thread->producer.en) {
        pre_msg_buf_new = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
        if (!pre_msg_buf_new) {
            zc_error("zlog_buf_new fail");
            goto err;
        }

        msg_buf_new = zlog_buf_new(buf_size_min, buf_size_max, "..." FILE_NEWLINE);
        if (!msg_buf_new) {
            zc_error("zlog_buf_new fail");
            goto err;
        }
    }

    if (a_thread->pre_msg_buf) {
        zlog_buf_del(a_thread->pre_msg_buf);
    }
    a_thread->pre_msg_buf = pre_msg_buf_new;

    if (a_thread->msg_buf) {
        zlog_buf_del(a_thread->msg_buf);
    }
    a_thread->msg_buf = msg_buf_new;

    return 0;
err:
	if (pre_msg_buf_new) zlog_buf_del(pre_msg_buf_new);
	if (msg_buf_new) zlog_buf_del(msg_buf_new);
	return -1;
}

int zlog_thread_rebuild_event(zlog_thread_t * a_thread, int time_cache_count)
{
	zlog_event_t *event_new = NULL;
	zc_assert(a_thread, -1);

	event_new = zlog_event_new(time_cache_count);
	if (!event_new) {
		zc_error("zlog_event_new fail");
		goto err;
	}

	zlog_event_del(a_thread->event);
	a_thread->event = event_new;
	return 0;
err:
	if (event_new) zlog_event_del(event_new);
	return -1;
}


/*******************************************************************************/

void zlog_thread_rebuild_producer(zlog_thread_t * thread, bool en)
{
    thread->producer.en = en;
    thread->producer.full_cnt = 0;
    atomic_init(&thread->producer.refcnt, 1);
    zc_debug("init %lx, refcnt %d", pthread_self(),
             atomic_load_explicit(&thread->producer.refcnt, memory_order_relaxed));
}
