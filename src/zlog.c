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

#include "fmacros.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/param.h>
#include <stdatomic.h>

#include "conf.h"
#include "category_table.h"
#include "record_table.h"
#include "mdc.h"
#include "unistd.h"
#include "zc_defs.h"
#include "rule.h"
#include "version.h"
#include "consumer.h"
#include "misc.h"
#include "fifo.h"

/*******************************************************************************/
extern char *zlog_git_sha1;
/*******************************************************************************/
static pthread_rwlock_t zlog_env_lock = PTHREAD_RWLOCK_INITIALIZER;
zlog_conf_t *zlog_env_conf;
static pthread_key_t zlog_thread_key;
static zc_hashtable_t *zlog_env_categories;
static zc_hashtable_t *zlog_env_records;
static zlog_category_t *zlog_default_category;
static size_t zlog_env_reload_conf_count;
static int zlog_env_is_init = 0;
static int zlog_env_init_version = 0;

static struct zlog_process_data process_data = {
	.share_mutex = PTHREAD_MUTEX_INITIALIZER,
};

/*******************************************************************************/
/* inner no need thread-safe */
static void zlog_fini_inner(void)
{
    if (!zlog_env_conf) {
        return;
    }

    if (zlog_env_conf->log_consumer.en) {
        log_consumer_destroy(process_data.logc);
    }

    /* pthread_key_delete(zlog_thread_key); */
    /* never use pthread_key_delete,
     * it will cause other thread can't release zlog_thread_t
     * after one thread call pthread_key_delete
     * also key not init will cause a core dump
     */

    if (zlog_env_categories)
        zlog_category_table_del(zlog_env_categories);
    zlog_env_categories = NULL;
    zlog_default_category = NULL;
    if (zlog_env_records)
        zlog_record_table_del(zlog_env_records);
    zlog_env_records = NULL;
    if (zlog_env_conf)
        zlog_conf_del(zlog_env_conf);
    zlog_env_conf = NULL;
    return;
}

static void zlog_clean_rest_thread(void)
{
	int rc;

	zlog_thread_t *a_thread;
	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) return;
	zlog_thread_del(a_thread);

	rc = pthread_setspecific(zlog_thread_key, NULL);
	if (rc) {
		zc_error("pthread_setspecific fail, rc[%d]", rc);
		return;
	}
	return;
}

static int zlog_init_inner_from_string(const char *config_string)
{
    int rc = 0;

    /* the 1st time in the whole process do init */
    if (zlog_env_init_version == 0) {
        /* clean up is done by OS when a thread call pthread_exit */
        rc = pthread_key_create(&zlog_thread_key, (void (*) (void *)) zlog_thread_del);
        if (rc) {
            zc_error("pthread_key_create fail, rc[%d]", rc);
            goto err;
        }

        /* if some thread do not call pthread_exit, like main thread
         * atexit will clean it
         */
        rc = atexit(zlog_clean_rest_thread);
        if (rc) {
            zc_error("atexit fail, rc[%d]", rc);
            goto err;
        }
        zlog_env_init_version++;
    } /* else maybe after zlog_fini() and need not create pthread_key */

    zlog_env_conf = zlog_conf_new_from_string(config_string);
    if (!zlog_env_conf) {
        zc_error("zlog_conf_new[%s] fail", config_string);
        goto err;
    }

    zlog_env_categories = zlog_category_table_new();
    if (!zlog_env_categories) {
        zc_error("zlog_category_table_new fail");
        goto err;
    }

    zlog_env_records = zlog_record_table_new();
    if (!zlog_env_records) {
        zc_error("zlog_record_table_new fail");
        goto err;
    }

    return 0;
err:
    zlog_fini_inner();
    return -1;
}
static int zlog_init_inner(const char *config)
{
	int rc = 0;

	/* the 1st time in the whole process do init */
	if (zlog_env_init_version == 0) {
		/* clean up is done by OS when a thread call pthread_exit */
		rc = pthread_key_create(&zlog_thread_key, (void (*) (void *)) zlog_thread_del);
		if (rc) {
			zc_error("pthread_key_create fail, rc[%d]", rc);
			goto err;
		}

		/* if some thread do not call pthread_exit, like main thread
		 * atexit will clean it 
		 */
		rc = atexit(zlog_clean_rest_thread);
		if (rc) {
			zc_error("atexit fail, rc[%d]", rc);
			goto err;
		}
		zlog_env_init_version++;
	} /* else maybe after zlog_fini() and need not create pthread_key */

	zlog_env_conf = zlog_conf_new(config);
	if (!zlog_env_conf) {
		zc_error("zlog_conf_new[%s] fail", config);
		goto err;
	}

	zlog_env_categories = zlog_category_table_new();
	if (!zlog_env_categories) {
		zc_error("zlog_category_table_new fail");
		goto err;
	}

	zlog_env_records = zlog_record_table_new();
	if (!zlog_env_records) {
		zc_error("zlog_record_table_new fail");
		goto err;
	}

	if (zlog_env_conf->log_consumer.en) {
		struct logc_create_arg arg = { .conf = zlog_env_conf };
		struct log_consumer *logc = log_consumer_create(&arg);
		if (!logc) {
			zc_error("logc fail");
			goto err;
		}

		process_data.logc = logc;
	}
	return 0;
err:
	zlog_fini_inner();
	return -1;
}

/*******************************************************************************/
XFUNC int zlog_init(const char *config)
{
	int rc;
	zc_debug("------zlog_init start------");
	zc_debug("------compile time[%s %s], version[%s]------", __DATE__, __TIME__, ZLOG_VERSION);

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (zlog_env_is_init) {
		zc_error("already init, use zlog_reload pls");
		goto err;
	}


	if (zlog_init_inner(config)) {
		zc_error("zlog_init_inner[%s] fail", config);
		goto err;
	}

	zlog_env_is_init = 1;
	zlog_env_init_version++;

	zc_debug("------zlog_init success end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	zc_error("------zlog_init fail end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}

XFUNC int zlog_init_from_string(const char *config_string)
{
    int rc;
    zc_debug("------zlog_init start------");
    zc_debug("------compile time[%s %s], version[%s]------", __DATE__, __TIME__, ZLOG_VERSION);

    rc = pthread_rwlock_wrlock(&zlog_env_lock);
    if (rc) {
        zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
        return -1;
    }

    if (zlog_env_is_init) {
        zc_error("already init, use zlog_reload pls");
        goto err;
    }


    if (zlog_init_inner_from_string(config_string)) {
        zc_error("zlog_init_inner[%s] fail", config_string);
        goto err;
    }

    zlog_env_is_init = 1;
    zlog_env_init_version++;

    zc_debug("------zlog_init success end------");
    rc = pthread_rwlock_unlock(&zlog_env_lock);
    if (rc) {
        zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
        return -1;
    }
    return 0;
    err:
    zc_error("------zlog_init fail end------");
    rc = pthread_rwlock_unlock(&zlog_env_lock);
    if (rc) {
        zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
        return -1;
    }
    return -1;
}

XFUNC int dzlog_init(const char *config, const char *cname)
{
	int rc = 0;
	zc_debug("------dzlog_init start------");
	zc_debug("------compile time[%s %s], version[%s]------",
			__DATE__, __TIME__, ZLOG_VERSION);

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (zlog_env_is_init) {
		zc_error("already init, use zlog_reload pls");
		goto err;
	}

	if (zlog_init_inner(config)) {
		zc_error("zlog_init_inner[%s] fail", config);
		goto err;
	}

	zlog_default_category = zlog_category_table_fetch_category(
				zlog_env_categories,
				cname,
				zlog_env_conf->rules);
	if (!zlog_default_category) {
		zc_error("zlog_category_table_fetch_category[%s] fail", cname);
		goto err;
	}

	zlog_env_is_init = 1;
	zlog_env_init_version++;

	zc_debug("------dzlog_init success end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	zc_error("------dzlog_init fail end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}
/*******************************************************************************/
XFUNC int zlog_reload(const char *config)
{
	int rc = 0;
	int i = 0;
	zlog_conf_t *new_conf = NULL;
	zlog_rule_t *a_rule;
	int c_up = 0;

	zc_debug("------zlog_reload start------");
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto quit;
	}

	/* use last conf file */
	if (config == NULL) config = zlog_env_conf->file;

	/* reach reload period */
	if (config == (char*)-1) {
		/* test again, avoid other threads already reloaded */
		if (zlog_env_reload_conf_count > zlog_env_conf->reload_conf_period) {
			config = zlog_env_conf->file;
		} else {
			/* do nothing, already done */
			goto quit;
		}
	}
    bool is_file = true;
    struct stat buffer;
    is_file = stat(config, &buffer) == 0;

    /* reset counter, whether automaticlly or mannually */
    zlog_env_reload_conf_count = 0;

    if (is_file) {
        new_conf = zlog_conf_new(config);
        if (!new_conf) {
            zc_error("zlog_conf_new fail");
            goto err;
        }
    } else {
        new_conf = zlog_conf_new_from_string(config);
        if (!new_conf) {
            zc_error("zlog_conf_new fail");
            goto err;
        }
    }

    if (zlog_env_conf->log_consumer.en) {
        /* ensure all data handled */
        log_consumer_queue_flush(process_data.logc);
    }

    zc_arraylist_foreach (new_conf->rules, i, a_rule) {
        zlog_rule_set_record(a_rule, zlog_env_records);
    }

    if (zlog_category_table_update_rules(zlog_env_categories, new_conf->rules)) {
        c_up = 0;
        zc_error("zlog_category_table_update fail");
        goto err;
    } else {
        c_up = 1;
    }

    if (c_up)
        zlog_category_table_commit_rules(zlog_env_categories);

    if (zlog_env_conf->log_consumer.en) {
        if (new_conf->log_consumer.en) {
            struct logc_create_arg arg = {
                .conf = new_conf,
            };
            struct log_consumer *logc = log_consumer_create(&arg);
            if (!logc) {
                goto err;
            }
            log_consumer_destroy(process_data.logc);
            zc_debug("reload zlog des %p, new %p\n", (void*)process_data.logc, (void*)logc);
            process_data.logc = logc;
        } else {
            log_consumer_destroy(process_data.logc);
            process_data.logc = NULL;
        }
    } else {
        if (new_conf->log_consumer.en) {
            struct logc_create_arg arg = {
                .conf = new_conf,
            };
            process_data.logc = log_consumer_create(&arg);
            if (!process_data.logc) {
                new_conf->log_consumer.en = false;
                goto err;
            }
        } else {
        }
    }

    zlog_conf_del(zlog_env_conf);
    zlog_env_conf = new_conf;
    zc_debug("------zlog_reload success, total init verison[%d] ------", zlog_env_init_version);
    zlog_env_init_version++;
    rc = pthread_rwlock_unlock(&zlog_env_lock);
    if (rc) {
        zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
        return -1;
    }
    return 0;
err:
    /* fail, roll back everything */
    zc_warn("zlog_reload fail, use old conf file, still working");
    if (new_conf)
        zlog_conf_del(new_conf);
    if (c_up)
        zlog_category_table_rollback_rules(zlog_env_categories);
    zc_error("------zlog_reload fail, total init version[%d] ------", zlog_env_init_version);
    rc = pthread_rwlock_unlock(&zlog_env_lock);
    if (rc) {
        zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
        return -1;
    }
    return -1;
quit:
    zc_debug("------zlog_reload do nothing------");
    rc = pthread_rwlock_unlock(&zlog_env_lock);
    if (rc) {
        zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
        return -1;
    }
    return 0;
}
/*******************************************************************************/
XFUNC int zlog_reload_from_string(const char *conf_string)
{
    return zlog_reload(conf_string);
}

/*******************************************************************************/
XFUNC void zlog_fini(void)
{
	int rc = 0;

	zc_debug("------zlog_fini start------");
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return;
	}

	if (!zlog_env_is_init) {
		zc_error("before finish, must zlog_init() or dzlog_init() first");
		goto exit;
	}

	zlog_fini_inner();
	zlog_env_is_init = 0;

exit:
	zc_debug("------zlog_fini end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}
/*******************************************************************************/
XFUNC zlog_category_t *zlog_get_category(const char *cname)
{
	int rc = 0;
	zlog_category_t *a_category = NULL;

	zc_assert(cname, NULL);
	zc_debug("------zlog_get_category[%s] start------", cname);
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return NULL;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		a_category = NULL;
		goto err;
	}

	a_category = zlog_category_table_fetch_category(
				zlog_env_categories,
				cname,
				zlog_env_conf->rules);
	if (!a_category) {
		zc_error("zlog_category_table_fetch_category[%s] fail", cname);
		goto err;
	}

	zc_debug("------zlog_get_category[%s] success, end------ ", cname);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return a_category;
err:
	zc_error("------zlog_get_category[%s] fail, end------ ", cname);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return NULL;
}

XFUNC int dzlog_set_category(const char *cname)
{
	int rc = 0;
	zc_assert(cname, -1);

	zc_debug("------dzlog_set_category[%s] start------", cname);
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto err;
	}

	zlog_default_category = zlog_category_table_fetch_category(
				zlog_env_categories,
				cname,
				zlog_env_conf->rules);
	if (!zlog_default_category) {
		zc_error("zlog_category_table_fetch_category[%s] fail", cname);
		goto err;
	}

	zc_debug("------dzlog_set_category[%s] end, success------ ", cname);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	zc_error("------dzlog_set_category[%s] end, fail------ ", cname);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}
/*******************************************************************************/
#define zlog_fetch_thread(a_thread, fail_goto) do {  \
	int rd = 0;  \
	a_thread = pthread_getspecific(zlog_thread_key);  \
	if (!a_thread) {  \
		a_thread = zlog_thread_new(zlog_env_init_version,  \
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max, \
				zlog_env_conf->time_cache_count, zlog_env_conf); \
		if (!a_thread) {  \
			zc_error("zlog_thread_new fail");  \
			goto fail_goto;  \
		}  \
  \
		rd = pthread_setspecific(zlog_thread_key, a_thread);  \
		if (rd) {  \
			zlog_thread_del(a_thread);  \
			zc_error("pthread_setspecific fail, rd[%d]", rd);  \
			goto fail_goto;  \
		}  \
	}  \
  \
	if (a_thread->init_version != zlog_env_init_version) {  \
        zlog_thread_rebuild_producer(a_thread, zlog_env_conf->log_consumer.en);\
		/* as mdc is still here, so can not easily del and new */ \
		rd = zlog_thread_rebuild_msg_buf(a_thread, \
				zlog_env_conf->buf_size_min, \
				zlog_env_conf->buf_size_max);  \
		if (rd) {  \
			zc_error("zlog_thread_resize_msg_buf fail, rd[%d]", rd);  \
			goto fail_goto;  \
		}  \
  \
		rd = zlog_thread_rebuild_event(a_thread, zlog_env_conf->time_cache_count);  \
		if (rd) {  \
			zc_error("zlog_thread_resize_msg_buf fail, rd[%d]", rd);  \
			goto fail_goto;  \
		}  \
		a_thread->init_version = zlog_env_init_version;  \
	}  \
} while (0)

/*******************************************************************************/
XFUNC int zlog_put_mdc(const char *key, const char *value)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	zc_assert(key, -1);
	zc_assert(value, -1);

	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto err;
	}

	zlog_fetch_thread(a_thread, err);

	if (zlog_mdc_put(a_thread->mdc, key, value)) {
		zc_error("zlog_mdc_put fail, key[%s], value[%s]", key, value);
		goto err;
	}

	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return -1;
}

XFUNC char *zlog_get_mdc(char *key)
{
	int rc = 0;
	char *value = NULL;
	zlog_thread_t *a_thread;

	zc_assert(key, NULL);

	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return NULL;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto err;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		zc_error("thread not found, maybe not use zlog_put_mdc before");
		goto err;
	}

	value = zlog_mdc_get(a_thread->mdc, key);
	if (!value) {
		zc_error("key[%s] not found in mdc", key);
		goto err;
	}

	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return value;
err:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return NULL;
	}
	return NULL;
}

XFUNC void zlog_remove_mdc(char *key)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	zc_assert(key, );

	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		zc_error("thread not found, maybe not use zlog_put_mdc before");
		goto exit;
	}

	zlog_mdc_remove(a_thread->mdc, key);

exit:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}

XFUNC void zlog_clean_mdc(void)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {;
		zc_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		zc_error("thread not found, maybe not use zlog_put_mdc before");
		goto exit;
	}

	zlog_mdc_clean(a_thread->mdc);

exit:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}

XFUNC int zlog_level_switch(zlog_category_t * category, int level)
{
    // This is NOT thread safe.
    memset(category->level_bitmap, 0x00, sizeof(category->level_bitmap));
    category->level_bitmap[level / 8] |= ~(0xFF << (8 - level % 8));
    memset(category->level_bitmap + level / 8 + 1, 0xFF,
	    sizeof(category->level_bitmap) -  level / 8 - 1);

    return 0;
}

/*******************************************************************************/
static void log_producer_send(zlog_thread_t *a_thread, zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen,
	long line, const int level,
	const char *format, va_list args)
{
    unsigned msg_size = 0;
    msg_size += msg_meta_size();
    va_list args_copy;
    va_copy(args_copy, args);
    int ret = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    if (ret < 0) {
        zc_error("failed to print to formatted_string ret %d", ret);
        return;
    }

    unsigned usr_str_size = ret + 1;
    unsigned usr_str_total_size_aligned = roundup(usr_str_size + msg_usr_str_size(), sizeof(long));
    msg_size += usr_str_total_size_aligned;

    struct msg_head *head = log_consumer_queue_reserve(process_data.logc, msg_size);
    if (!head) {
        zc_error("fifo no enough mem %u > free %u", msg_size, fifo_unused(process_data.logc->event.queue));
        a_thread->producer.full_cnt++;
        return;
    }

    struct msg_meta *meta = (struct msg_meta *)head->data;
    struct msg_usr_str *usr_str = (struct msg_usr_str *)(head->data + msg_meta_size());

    ret = vsnprintf(usr_str->formatted_string, usr_str_size, format, args);
    if (ret < 0) {
        /* should not happend */
        zc_error("failed to print to formatted_string ret %d", ret);
        goto discard;
    }
    if (ret >= usr_str_size) {
        zc_error("warning truncated");
        usr_str->formatted_string[usr_str_size - 1] = '\0';
    }
    usr_str->formatted_string_size = usr_str_size;
    usr_str->total_size = usr_str_total_size_aligned;
    usr_str->type.val = MSG_TYPE_USR_STR;

    meta->type.val = MSG_TYPE_META;
    meta->category = category;
    meta->file = file;
    meta->filelen = filelen;
    meta->func = func;
    meta->funclen = funclen;
    meta->line = line;
    meta->level = level;
    meta->thread = a_thread;
    ret = clock_gettime(CLOCK_REALTIME, &meta->ts); /* todo: CLOCK_MONOTONIC  ? */
    if (ret) {
        zc_error("failed to get ts ret %d", ret);
        goto discard;
    }

    atomic_fetch_add(&a_thread->producer.refcnt, 1);
    log_consumer_queue_commit_signal(process_data.logc, head, false);

    return;

discard:
    atomic_fetch_add(&a_thread->producer.refcnt, 1);
    log_consumer_queue_commit_signal(process_data.logc, head, true);
}

static void _log(zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen,
	long line, const int level,
	const char *format, va_list args)
{
	zlog_thread_t *a_thread;

	/* The bitmap determination here is not under the protection of rdlock.
	 * It may be changed by other CPU by zlog_reload() halfway.
	 *
	 * Old or strange value may be read here,
	 * but it is safe, the bitmap is valid as long as category exist,
	 * And will be the right value after zlog_reload()
	 *
	 * For speed up, if one log will not be output,
	 * There is no need to aquire rdlock.
	 */
	pthread_rwlock_rdlock(&zlog_env_lock);
	
	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto exit;
	}

    if (!category) {
        goto exit;
    }

	if (zlog_category_needless_level(category, level)) {
		goto exit;
	}

	zlog_fetch_thread(a_thread, exit);

    if (zlog_env_conf->log_consumer.en) {
        log_producer_send(a_thread, category, file, filelen, func, funclen, line, level, format,
                          args);
    } else {
        zlog_event_set_fmt(a_thread->event, category->name, category->name_len,
            file, filelen, func, funclen, line, level,
            format, args);
        if (zlog_category_output(category, a_thread, NULL)) {
            zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line);
            goto exit;
        }
    }

	if (zlog_env_conf->reload_conf_period &&
		++zlog_env_reload_conf_count > zlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&zlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&zlog_env_lock);
	/* will be wrlock, so after unlock */
	if (zlog_reload((char *)-1)) {
		zc_error("reach reload-conf-period but zlog_reload fail, zlog-chk-conf [file] see detail");
	}
	return;
}

XFUNC void vzlog(zlog_category_t * category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const char *format, va_list args)
{
    _log(category, file, filelen, func, funclen, line, level, format, args);
}

XFUNC void hzlog(zlog_category_t *category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	zlog_thread_t *a_thread;

	pthread_rwlock_rdlock(&zlog_env_lock);
	
	if (zlog_category_needless_level(category, level)) goto exit;

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto exit;
	}

	zlog_fetch_thread(a_thread, exit);

	zlog_event_set_hex(a_thread->event,
		category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		buf, buflen);

	if (zlog_category_output(category, a_thread, NULL)) {
		zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}

	if (zlog_env_conf->reload_conf_period &&
		++zlog_env_reload_conf_count > zlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&zlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&zlog_env_lock);
	/* will be wrlock, so after unlock */
	if (zlog_reload((char *)-1)) {
		zc_error("reach reload-conf-period but zlog_reload fail, zlog-chk-conf [file] see detail");
	}
	return;
}

/*******************************************************************************/
/* for speed up, copy from vzlog */
XFUNC void vdzlog(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const char *format, va_list args)
{
    _log(zlog_default_category, file, filelen, func, funclen, line, level, format, args);
}

XFUNC void hdzlog(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	zlog_thread_t *a_thread;

	pthread_rwlock_rdlock(&zlog_env_lock);
	
	if (zlog_category_needless_level(zlog_default_category, level)) goto exit;

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!zlog_default_category) {
		zc_error("zlog_default_category is null,"
			"dzlog_init() or dzlog_set_cateogry() is not called above");
		goto exit;
	}

	zlog_fetch_thread(a_thread, exit);

	zlog_event_set_hex(a_thread->event,
		zlog_default_category->name, zlog_default_category->name_len,
		file, filelen, func, funclen, line, level,
		buf, buflen);

	if (zlog_category_output(zlog_default_category, a_thread, NULL)) {
		zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}

	if (zlog_env_conf->reload_conf_period &&
		++zlog_env_reload_conf_count > zlog_env_conf->reload_conf_period ) {
		/* under the protection of lock read env conf */
		goto reload;
	}

exit:
	pthread_rwlock_unlock(&zlog_env_lock);
	return;
reload:
	pthread_rwlock_unlock(&zlog_env_lock);
	/* will be wrlock, so after unlock */
	if (zlog_reload((char *)-1)) {
		zc_error("reach reload-conf-period but zlog_reload fail, zlog-chk-conf [file] see detail");
	}
	return;
}

/*******************************************************************************/
XFUNC void zlog(zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen,
	long line, const int level,
	const char *format, ...)
{
	va_list args;

	va_start(args, format);
    _log(category, file, filelen, func, funclen, line, level, format, args);
	va_end(args);
}

/*******************************************************************************/
XFUNC void dzlog(const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const char *format, ...)
{
	va_list args;

	va_start(args, format);
    _log(zlog_default_category, file, filelen, func, funclen, line, level, format, args);
	va_end(args);
}

/*******************************************************************************/
XFUNC void zlog_profile(void)
{
	int rc = 0;
	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return;
	}
	zc_warn("------zlog_profile start------ ");
	zc_warn("is init:[%d]", zlog_env_is_init);
	zc_warn("init version:[%d]", zlog_env_init_version);
	zlog_conf_profile(zlog_env_conf, ZC_WARN);
	zlog_record_table_profile(zlog_env_records, ZC_WARN);
	zlog_category_table_profile(zlog_env_categories, ZC_WARN);
	if (zlog_default_category) {
		zc_warn("-default_category-");
		zlog_category_profile(zlog_default_category, ZC_WARN);
	}
	zc_warn("------zlog_profile end------ ");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return;
	}
	return;
}
/*******************************************************************************/
XFUNC int zlog_set_record(const char *rname, zlog_record_fn record_output)
{
	int rc = 0;
	int rd = 0;
	zlog_rule_t *a_rule;
	zlog_record_t *a_record;
	int i = 0;

	zc_assert(rname, -1);
	zc_assert(record_output, -1);

	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return -1;
	}

	if (!zlog_env_is_init) {
		zc_error("never call zlog_init() or dzlog_init() before");
		goto zlog_set_record_exit;
	}

	a_record = zlog_record_new(rname, record_output);
	if (!a_record) {
		rc = -1;
		zc_error("zlog_record_new fail");
		goto zlog_set_record_exit;
	}

	rc = zc_hashtable_put(zlog_env_records, a_record->name, a_record);
	if (rc) {
		zlog_record_del(a_record);
		zc_error("zc_hashtable_put fail");
		goto zlog_set_record_exit;
	}

	zc_arraylist_foreach(zlog_env_conf->rules, i, a_rule) {
		zlog_rule_set_record(a_rule, zlog_env_records);
	}

      zlog_set_record_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}
	return rc;
}
/*******************************************************************************/
XFUNC int zlog_level_enabled(zlog_category_t *category, const int level)
{
	int enable = 0;

	pthread_rwlock_rdlock(&zlog_env_lock);
	enable = category && ((zlog_category_needless_level(category, level) == 0));
	pthread_rwlock_unlock(&zlog_env_lock);
	
	return enable;
}

int dzlog_level_enabled(const int level)
{
	return zlog_level_enabled(zlog_default_category, level);
}

const char *zlog_version(void) { return ZLOG_VERSION; }
