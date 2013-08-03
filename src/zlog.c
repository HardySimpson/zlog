/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#include "fmacros.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>


/*******************************************************************************/
static pthread_rwlock_t zlog_env_lock = PTHREAD_RWLOCK_INITIALIZER;
static zlog_conf_t *zlog_env_conf;
static zc_hashtable_t *zlog_env_records;
static int zlog_env_inited = 0;
static int zlog_env_version = -1;

/* 
 * using posix pthread_key cleaup method when user's thread call pthread_exit(),
 * also at the same time, put all threads in a list,
 * explicitly traverse and del them at zlog_fini()
 */
static pthread_key_t zlog_thread_key;
static zc_arraylist_t *zlog_env_threads;

/*******************************************************************************/
static void zlog_fini_inner(void)
{
	/* pthread_key_delete(zlog_thread_key); */
	/* never use pthread_key_delete,
	 * it will cause other thread can't release zlog_thread_t 
	 * after one thread call pthread_key_delete
	 * also key not init will cause a core dump
	 */
	
	if (zlog_env_threads) zc_arraylist_del(zlog_env_threads);
	zlog_env_threads = NULL;
	if (zlog_env_records) zlog_record_table_del(zlog_env_records);
	zlog_env_records = NULL;
	if (zlog_env_rotater) zlog_rotater_del(zlog_env_rotater);
	zlog_env_rotater = NULL;
	if (zlog_env_conf) zlog_conf_del(zlog_env_conf);
	zlog_env_conf = NULL;
	return;
}

static void zlog_thread_del_in_list(zlog_thread_t *a_thread)
{
	int rc;
	rc = zc_arraylist_set(zlog_env_threads, a_thread->idx, NULL);
	if (rc) { zc_error("zc_arraylist_set fail");
	return;
}

/*******************************************************************************/
int zlog_init(const char *confpath)
{
	int rc;
	zc_debug("------zlog_init start------");
	zc_debug("------compile time[%s %s], version[%s]------", __DATE__, __TIME__, ZLOG_VERSION);

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return -1; }

	if (zlog_env_inited) { zc_error("already init, use zlog_reload pls"); goto err; }

	if (zlog_env_version == -1) {
		rc = pthread_key_create(&zlog_thread_key, (void (*) (void *)) zlog_thread_del_in_list);
		if (rc) { zc_error("pthread_key_create fail, rc[%d]", rc); goto err; }
		zlog_env_version = 0;
	} /* else maybe after zlog_fini() and need not create pthread_key again */

	zlog_env_conf = zlog_conf_new(confpath);
	if (!zlog_env_conf) { zc_error("zlog_conf_new[%s] fail", confpath); goto cleanup; }

	zlog_env_rotater = zlog_rotater_new(zlog_env_conf->rotate_lock_file);
	if (!zlog_env_conf) { zc_error("zlog_rotater_new[%s] fail", zlog_env_conf->rotate_lock_file); goto cleanup; }

	zlog_env_records = zlog_record_table_new();
	if (!zlog_env_records) { zc_error("zlog_record_table_new fail"); goto cleanup; }

	zlog_env_threads = zc_arraylist_new(0);
	if (!zlog_env_threads) { zc_error("zc_arraylist_new fail"); goto cleanup; }
	zc_arraylist_set_del(zlog_env_threads, zlog_thread_del);

	zlog_env_inited = 1;
	zlog_env_version++;

	zc_debug("------zlog_init success end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return 0;

cleanup:
	zlog_fini_inner();
err:
	zc_error("------zlog_init fail end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return -1;
}

/*******************************************************************************/
int zlog_reload(const char *confpath)
{
	int rc = 0;
	zlog_conf_t *new_conf = NULL;

	zc_debug("------zlog_reload start------");
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return -1; }

	if (!zlog_env_inited) { zc_error("never call zlog_init() or dzlog_init() before"); goto exit; }

	/* use last conf file */
	if (confpath == NULL) confpath = zlog_env_conf->file;

	new_conf = zlog_conf_new(confpath);
	if (!new_conf) { zc_error("zlog_conf_new fail"); goto err; }

	if (STRCMP(new_conf->rotater_lock_file, !=, zlog_env_conf->rotater_lock_file)) {
		rc = zlog_rotater_reload(new_conf->rotater_lock_file);
		if (rc) { zc_error("zlog_rotater_reload fail"); goto err; }
	}

	/* zlog_env_records don't change here, as it's info comes from user's api */

	zlog_env_conf = new_conf;
	zlog_env_version++;
	zc_debug("------zlog_reload success, total verison[%d] ------", zlog_env_version);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return 0;

err:
	/* fail, roll back everything */
	zc_warn("zlog_reload fail, use old conf file, still working");
	if (new_conf) zlog_conf_del(new_conf);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return -1;
exit:
	zc_debug("------zlog_reload do nothing------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return 0;
}
/*******************************************************************************/
void zlog_fini(void)
{
	int rc = 0;

	zc_debug("------zlog_fini start------");
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return; }

	if (!zlog_env_inited) { zc_error("before finish, must zlog_init() or dzlog_init() fisrt"); goto exit; }

	zlog_fini_inner();
	zlog_env_inited = 0;

exit:
	zc_debug("------zlog_fini end------");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return; }
	return;
}
/*******************************************************************************/

/* should be called under the protect of wrlock, as it modifies the zlog_env_threads.
 * slow, will not be called when do logging, but should be called when no category exist.
 */

zlog_thread_t *zlog_fetch_thread(void)
{  
	int rc = 0;  
	zlog_thread_t *a_thread;

	a_thread = pthread_getspecific(zlog_thread_key);  
	if (!a_thread) {  
		a_thread = zlog_thread_new(zlog_env_conf, zlog_env_records, zlog_env_version);
		if (!a_thread) {  zc_error("zlog_thread_new fail"); return NULL;}  
  
		rc = pthread_setspecific(zlog_thread_key, a_thread);  
		if (rc) {  
			zlog_thread_del(a_thread);  
			zc_error("pthread_setspecific fail, rc[%d]", rc);  
			return NULL;
		} 

		/* add thread to list, and set itself the index of list */
		rc = zc_arraylist_add(zlog_env_threads, a_thread, &(a_thread->idx));
		if (rc) {  
			zlog_thread_del(a_thread);  
			zc_error("zc_arraylist_add fail, rc[%d]", rc);  
			return NULL;
		} 
	}  
  
	if (a_thread->version != zlog_env_version) {  
		rc = zlog_thread_update(a_thread, zlog_env_conf, zlog_env_records, zlog_env_version); 
		if (rc) {  zc_error("zlog_thread_reload fail, rc[%d]", rc);  return NULL; }  
	}  
	return a_thread;
}

/*******************************************************************************/
zlog_category_t *zlog_get_category(const char *cname)
{
	int rc = 0;
	zlog_category_t * a_category = NULL;
	zlog_thread_t * a_thread;

	zc_assert(cname, NULL);
	zc_debug("------zlog_get_category[%s] start------", cname);
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return NULL;
	}

	if (!zlog_env_inited) { zc_error("never call zlog_init() or dzlog_init() before"); goto err; }

	a_thread = zlog_fetch_thread();
	if(!a_thread) { zc_error("zlog_fetch_thread fail"); goto err; }

	a_category = zlog_thread_fetch_category(a_thread, cname, zlog_env_records);
	if (!a_category) { zc_error("zlog_thread_fetch_category[%s] fail", cname); goto err; }

	zc_debug("------zlog_get_category[%s] success, end------ ", cname);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
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

/*******************************************************************************/
int zlog_put_mdc(const char *key, const char *value)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	zc_assert(key, -1);
	zc_assert(value, -1);

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return -1; }

	if (!zlog_env_inited) { zc_error("never call zlog_init() or dzlog_init() before"); goto err; }

	a_thread = zlog_fetch_thread();
	if(!a_thread) { zc_error("zlog_fetch_thread fail"); goto err; }

	rc = zlog_mdc_put(a_thread->mdc, key, value);
	if (rc) { zc_error("zlog_mdc_put fail, key[%s], value[%s]", key, value); goto err; }

	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return 0;
err:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return -1;
}

char *zlog_get_mdc(const char *key)
{
	int rc = 0;
	char *value = NULL;
	zlog_thread_t *a_thread;

	zc_assert(key, NULL);

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return NULL; }

	if (!zlog_env_inited) { zc_error("never call zlog_init() or dzlog_init() before"); goto err; }

	a_thread = zlog_fetch_thread();
	if(!a_thread) { zc_error("zlog_fetch_thread fail"); goto err; }

	value = zlog_mdc_get(a_thread->mdc, key);
	if (!value) { zc_error("key[%s] not found in mdc", key); goto err; }

	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return NULL; }
	return value;
err:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return NULL; }
	return NULL;
}

void zlog_remove_mdc(const char *key)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	zc_assert(key, );

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return; }

	if (!zlog_env_inited) { zc_error("never call zlog_init() or dzlog_init() before"); goto exit; }

	a_thread = zlog_fetch_thread();
	if(!a_thread) { zc_error("zlog_fetch_thread fail"); goto err; }

	zlog_mdc_remove(a_thread->mdc, key);

exit:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return; } return;
}

void zlog_clean_mdc(void)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return; }

	if (!zlog_env_inited) { zc_error("never call zlog_init() or dzlog_init() before"); goto exit; }

	a_thread = zlog_fetch_thread();
	if(!a_thread) { zc_error("zlog_fetch_thread fail"); goto err; }

	zlog_mdc_clean(a_thread->mdc);

exit:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return; }
	return;
}

/*******************************************************************************/
void vzlog(zlog_category_t * category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const char *format, va_list args)
{
	int rc;

	if (zlog_category_without_level(category, level)) return;

	zlog_event_set_fmt(category->event, category->name, 
		file, filelen, func, funclen, line, level,
		format, args);

	rc = zlog_category_output(category);
	if (rc) { zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line); return; }

	if (a_category->version != zlog_env_version) {
		rc = pthread_rwlock_rdlock(&zlog_env_lock);
		if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return; }

		rc = zlog_thread_update(a_thread, zlog_env_conf, zlog_env_records, zlog_env_version); 
		if (rc) { zc_error("zlog_thread_reload fail, rc[%d]", rc); goto exit; }  

exit:
		rc = pthread_rwlock_unlock(&zlog_env_lock);
		if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return; }
	}

	return;
}

void hzlog(zlog_category_t *category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	int rc;

	if (zlog_category_without_level(category, level)) return;

	zlog_event_set_hex(a_thread->event,
		category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		buf, buflen);

	rc = zlog_category_output(category);
	if (rc) { zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line); return; }

	if (a_category->version != zlog_env_version) {
		rc = pthread_rwlock_rdlock(&zlog_env_lock);
		if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return; }

		rc = zlog_thread_update(a_thread, zlog_env_conf, zlog_env_records, zlog_env_version); 
		if (rc) { zc_error("zlog_thread_reload fail, rc[%d]", rc); goto exit; }  

exit:
		rc = pthread_rwlock_unlock(&zlog_env_lock);
		if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return; }
	}

	return;
}

/*******************************************************************************/
void zlog(zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen,
	long line, const int level,
	const char *format, ...)
{
	int rc;
	va_list args;

	if (zlog_category_without_level(category, level)) return;

	va_start(args, format);
	zlog_event_set_fmt(category->event, category->name, 
		file, filelen, func, funclen, line, level,
		format, args);
	va_end(args);

	rc = zlog_category_output(category);
	if (rc) { zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line); return; }

	if (a_category->version != zlog_env_version) {
		rc = pthread_rwlock_rdlock(&zlog_env_lock);
		if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return; }

		rc = zlog_thread_update(a_thread, zlog_env_conf, zlog_env_records, zlog_env_version); 
		if (rc) { zc_error("zlog_thread_reload fail, rc[%d]", rc); goto exit; }  

exit:
		rc = pthread_rwlock_unlock(&zlog_env_lock);
		if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return; }
	}

	return;
}
/*******************************************************************************/
void zlog_profile(void)
{
	int rc = 0;
	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc); return; }
	zc_warn("------zlog_profile start------ ");
	zc_warn("is init:[%d]", zlog_env_inited);
	zc_warn("init version:[%d]", zlog_env_version);
	zlog_conf_profile(zlog_env_conf, ZC_WARN);
	zlog_record_table_profile(zlog_env_records, ZC_WARN);
	zlog_category_table_profile(zlog_env_categories, ZC_WARN);
	zc_warn("------zlog_profile end------ ");
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return; }
	return;
}
/*******************************************************************************/
int zlog_set_record(const char *rname, zlog_record_fn record_output)
{
	int rc = 0;
	int rd = 0;
	zlog_rule_t *a_rule;
	zlog_record_t *a_record;
	int i = 0;

	zc_assert(rname, -1);
	zc_assert(record_output, -1);

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_rdlock fail, rc[%d]", rc); return -1; }

	if (!zlog_env_inited) { zc_error("never call zlog_init() or dzlog_init() before"); goto err; }

	a_record = zlog_record_new(rname, record_output);
	if (!a_record) { zc_error("zlog_record_new fail"); goto err; }

	rc = zc_hashtable_put(zlog_env_records, a_record->name, a_record);
	if (rc) {
		zlog_record_del(a_record);
		zc_error("zc_hashtable_put fail");
		goto err;
	}

	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return 0;
err:
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) { zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc); return -1; }
	return -1;
}
