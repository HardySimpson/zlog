/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * The zlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The zlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the zlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "conf.h"
#include "category_table.h"
#include "thread_list.h"
#include "record_table.h"
#include "mdc.h"
#include "zc_defs.h"
#include "rule.h"

/*******************************************************************************/
static pthread_rwlock_t zlog_env_lock = PTHREAD_RWLOCK_INITIALIZER;
static zlog_conf_t *zlog_env_conf;
static pthread_key_t zlog_thread_key;
static zc_arraylist_t *zlog_env_threads;
static zc_hashtable_t *zlog_env_categories;
static zc_hashtable_t *zlog_env_records;
static zlog_category_t *zlog_default_category;
static volatile sig_atomic_t zlog_env_reload_conf_count;
static int zlog_env_init_flag = -1;
/*******************************************************************************/
/* inner no need thread-safe */
static void zlog_fini_inner(void)
{
	pthread_key_delete(zlog_thread_key);
	if (zlog_env_threads) zlog_thread_list_del(zlog_env_threads);
	if (zlog_env_categories) zlog_category_table_del(zlog_env_categories);
	if (zlog_env_records) zlog_record_table_del(zlog_env_records);
	if (zlog_env_conf) zlog_conf_del(zlog_env_conf);
	zlog_env_threads = NULL;
	zlog_env_categories = NULL;
	zlog_env_conf = NULL;
	zlog_default_category = NULL;
	return;
}

static int zlog_init_inner(char *confpath)
{
	int rc;
	zlog_thread_t *a_thread;

	zlog_env_conf = zlog_conf_new(confpath);
	if (!zlog_env_conf) {
		zc_error("zlog_conf_new[%s] fail", confpath);
		goto err;
	}

	zlog_env_categories = zlog_category_table_new();
	if (!zlog_env_categories) {
		zc_error("zlog_category_table_new fail");
		goto err;
	}

	/* clean up is done by arraylist of threads, not by OS */
	rc = pthread_key_create(&zlog_thread_key, NULL);
	if (rc) {
		zc_error("pthread_key_create fail, rc[%d]", rc);
		goto err;
	}

	zlog_env_threads = zlog_thread_list_new();
	if (!zlog_env_threads) {
		zc_error("zlog_thread_list_new fail");
		goto err;
	}

	/* create zlog_thread in init thread */
	a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
			zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
	if (!a_thread) {
		zc_error("zlog_thread_list_new_thread fail");
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

/*******************************************************************************/
int zlog_init(char *confpath)
{
	int rc;
	zc_debug("------zlog_init start, compile time[%s %s]------", __DATE__, __TIME__);

	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (zlog_env_init_flag > 0) {
		zc_error("already init, use zlog_reload pls");
		goto err;
	}

	if (zlog_init_inner(confpath)) {
		zc_error("zlog_init_inner[%s] fail", confpath);
		goto err;
	}

	zlog_env_init_flag = 1;

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

int dzlog_init(char *confpath, char *cname)
{
	int rc = 0;
	zc_debug("------dzlog_init start, compile time[%s %s]------", __DATE__, __TIME__);
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (zlog_env_init_flag > 0) {
		zc_error("already init, use zlog_reload pls");
		goto err;
	}

	if (zlog_init_inner(confpath)) {
		zc_error("zlog_init_inner[%s] fail", confpath);
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

	zlog_env_init_flag = 1;

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
int zlog_reload(char *confpath)
{
	int rc = 0;
	int i = 0;
	zlog_conf_t *new_conf = NULL;
	zlog_rule_t *a_rule;
	int t_up = 0;
	int c_up = 0;

	zc_debug("------zlog_reload start------");
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (zlog_env_init_flag < 0) {
		zc_error("never zlog_init before");
		goto quit;
	}

	/* use last conf file */
	if (confpath == NULL) confpath = zlog_env_conf->file;

	/* reach reload period */
	if (confpath == (char*)-1) {
		/* test again, avoid other threads already reloaded */
		if (zlog_env_reload_conf_count > zlog_env_conf->reload_conf_period) {
			confpath = zlog_env_conf->file;
		} else {
			/* do nothing, already done */
			goto quit;
		}
	}

	/* reset counter, whether automaticlly or mannually */
	zlog_env_reload_conf_count = 0;

	new_conf = zlog_conf_new(confpath);
	if (!new_conf) {
		zc_error("zlog_conf_new fail");
		goto err;
	}

	zc_arraylist_foreach(new_conf->rules, i, a_rule) {
		zlog_rule_set_record(a_rule, zlog_env_records);
	}

	if (zlog_category_table_update_rules(zlog_env_categories, new_conf->rules)) {
		c_up = 0;
		zc_error("zlog_category_table_update fail");
		goto err;
	} else {
		c_up = 1;
	}

	if ((new_conf->buf_size_min != zlog_env_conf->buf_size_min) ||
		(new_conf->buf_size_max != zlog_env_conf->buf_size_max) ) {
		if (zlog_thread_list_update_msg_buf(zlog_env_threads,
				new_conf->buf_size_min, new_conf->buf_size_max)) {
			t_up = 0;
			zc_error("zlog_thread_list_update_msg_buf fail");
			goto err;
		} else {
			t_up = 1;
		}
	} else {
		t_up = 0;
	}

	zlog_env_init_flag++;

	if (c_up) zlog_category_table_commit_rules(zlog_env_categories);
	if (t_up) zlog_thread_list_commit_msg_buf(zlog_env_threads);
	zlog_conf_del(zlog_env_conf);
	zlog_env_conf = new_conf;
	zc_debug("------zlog_reload [%d] times end, success------", zlog_env_init_flag);
	rc = pthread_rwlock_unlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_unlock fail, rc=[%d]", rc);
		return -1;
	}
	return 0;
err:
	/* fail, roll back everything */
	zc_warn("zlog_reload fail, use old conf file, still working");
	if (new_conf) zlog_conf_del(new_conf);
	if (c_up) zlog_category_table_rollback_rules(zlog_env_categories);
	if (t_up) zlog_thread_list_rollback_msg_buf(zlog_env_threads);
	zc_error("------zlog_reload [%d] times end, fail------", zlog_env_init_flag);
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
void zlog_fini(void)
{
	int rc = 0;

	zc_debug("------zlog_fini start------");
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return;
	}

	if (zlog_env_init_flag < 0) {
		zc_error("before finish, must zlog_init fisrt");
		goto exit;
	}

	zlog_fini_inner();
	zlog_env_init_flag = 0;

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
zlog_category_t *zlog_get_category(char *cname)
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

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
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

	zc_debug("------zlog_get_category[%s] fail, end------ ", cname);
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

int dzlog_set_category(char *cname)
{
	int rc = 0;
	zc_assert(cname, -1);

	zc_debug("------dzlog_set_category[%s] start------", cname);
	rc = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return -1;
	}

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
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
int zlog_put_mdc(char *key, char *value)
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

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto err;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		rc = pthread_rwlock_unlock(&zlog_env_lock);
		if (rc) {
			zc_error("pthread_rwlock_unlock fail, rc[%d]", rc);
			goto err;
		}
		rc = pthread_rwlock_wrlock(&zlog_env_lock);
		if (rc) {
			zc_error("pthread_rwlock_unlock fail, rc[%d]", rc);
			goto err;
		}
		/* change to wrlock, create and associate */
		a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
		if (!a_thread) {
			zc_error("zlog_thread_list_new_thread fail");
			goto err;
		}
	}

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

char *zlog_get_mdc(char *key)
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

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
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

void zlog_remove_mdc(char *key)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	zc_assert(key, );

	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return;
	}

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
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

void zlog_clean_mdc(void)
{
	int rc = 0;
	zlog_thread_t *a_thread;

	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {;
		zc_error("pthread_rwlock_rdlock fail, rc[%d]", rc);
		return;
	}

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
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

/*******************************************************************************/
void vzlog(zlog_category_t * category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const char *format, va_list args)
{
	zlog_thread_t *a_thread;

	pthread_rwlock_rdlock(&zlog_env_lock);

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto exit;
	}

	if (!zlog_category_should_ouput(category, level)) {
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		pthread_rwlock_unlock(&zlog_env_lock);

		pthread_rwlock_wrlock(&zlog_env_lock);
		/* change to wrlock, create and associate */
		a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
		if (!a_thread) {
			zc_error("zlog_thread_list_new_thread fail");
			goto exit;
		}
	}

	zlog_event_set_fmt(a_thread->event,
		category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);

	if (zlog_category_output(category, a_thread)) {
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

void hzlog(zlog_category_t *category,
	const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	zlog_thread_t *a_thread;

	pthread_rwlock_rdlock(&zlog_env_lock);

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto exit;
	}

	if (!zlog_category_should_ouput(category, level)) {
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		pthread_rwlock_unlock(&zlog_env_lock);

		pthread_rwlock_wrlock(&zlog_env_lock);
		/* change to wrlock, create and associate */
		a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
		if (!a_thread) {
			zc_error("zlog_thread_list_new_thread fail");
			goto exit;
		}
	}

	zlog_event_set_hex(a_thread->event,
		category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		buf, buflen);

	if (zlog_category_output(category, a_thread)) {
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
void vdzlog(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const char *format, va_list args)
{
	zlog_thread_t *a_thread;

	pthread_rwlock_rdlock(&zlog_env_lock);

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!zlog_default_category) {
		zc_error("zlog_default_category is null,"
			"dzlog_init() or dzlog_set_cateogry() is not called above");
		goto exit;
	}

	if (!zlog_category_should_ouput(zlog_default_category, level)) {
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		pthread_rwlock_unlock(&zlog_env_lock);

		pthread_rwlock_wrlock(&zlog_env_lock);
		/* change to wrlock, create and associate */
		a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
		if (!a_thread) {
			zc_error("zlog_thread_list_new_thread fail");
			goto exit;
		}
	}

	zlog_event_set_fmt(a_thread->event,
		zlog_default_category->name, zlog_default_category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);

	if (zlog_category_output(zlog_default_category, a_thread)) {
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

void hdzlog(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	zlog_thread_t *a_thread;

	pthread_rwlock_rdlock(&zlog_env_lock);

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!zlog_default_category) {
		zc_error("zlog_default_category is null,"
			"dzlog_init() or dzlog_set_cateogry() is not called above");
		goto exit;
	}

	if (!zlog_category_should_ouput(zlog_default_category, level)) {
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		pthread_rwlock_unlock(&zlog_env_lock);

		pthread_rwlock_wrlock(&zlog_env_lock);
		/* change to wrlock, create and associate */
		a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
		if (!a_thread) {
			zc_error("zlog_thread_list_new_thread fail");
			goto exit;
		}
	}

	zlog_event_set_hex(a_thread->event,
		zlog_default_category->name, zlog_default_category->name_len,
		file, filelen, func, funclen, line, level,
		buf, buflen);

	if (zlog_category_output(zlog_default_category, a_thread)) {
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
void zlog(zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen,
	long line, const int level,
	const char *format, ...)
{
	zlog_thread_t *a_thread;
	va_list args;

	pthread_rwlock_rdlock(&zlog_env_lock);

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto exit;
	}

	if (!zlog_category_should_ouput(category, level)) {
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		pthread_rwlock_unlock(&zlog_env_lock);

		pthread_rwlock_wrlock(&zlog_env_lock);
		/* change to wrlock, create and associate */
		a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
		if (!a_thread) {
			zc_error("zlog_thread_list_new_thread fail");
			goto exit;
		}
	}

	va_start(args, format);
	zlog_event_set_fmt(a_thread->event,
		category->name, category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);
	if (zlog_category_output(category, a_thread)) {
		zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}
	va_end(args);

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
void dzlog(const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const char *format, ...)
{
	zlog_thread_t *a_thread;
	va_list args;

	pthread_rwlock_rdlock(&zlog_env_lock);

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!zlog_default_category) {
		zc_error("zlog_default_category is null,"
			"dzlog_init() or dzlog_set_cateogry() is not called above");
		goto exit;
	}

	if (!zlog_category_should_ouput(zlog_default_category, level)) {
		goto exit;
	}

	a_thread = pthread_getspecific(zlog_thread_key);
	if (!a_thread) {
		pthread_rwlock_unlock(&zlog_env_lock);

		pthread_rwlock_wrlock(&zlog_env_lock);
		/* change to wrlock, create and associate */
		a_thread = zlog_thread_list_new_thread(zlog_env_threads, zlog_thread_key,
				zlog_env_conf->buf_size_min, zlog_env_conf->buf_size_max);
		if (!a_thread) {
			zc_error("zlog_thread_list_new_thread fail");
			goto exit;
		}
	}

	va_start(args, format);
	zlog_event_set_fmt(a_thread->event,
		zlog_default_category->name, zlog_default_category->name_len,
		file, filelen, func, funclen, line, level,
		format, args);

	if (zlog_category_output(zlog_default_category, a_thread)) {
		zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto exit;
	}
	va_end(args);

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
void zlog_profile(void)
{
	int rc = 0;
	rc = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rc) {
		zc_error("pthread_rwlock_wrlock fail, rc[%d]", rc);
		return;
	}
	zc_warn("------zlog_profile start------ ");
	zc_warn("init_flag:[%d]", zlog_env_init_flag);
	zlog_conf_profile(zlog_env_conf, ZC_WARN);
	zlog_record_table_profile(zlog_env_records, ZC_WARN);
	zlog_thread_list_profile(zlog_env_threads, ZC_WARN);
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
int zlog_set_record(char *name, zlog_record_fn record_output)
{
	int rc = 0;
	int rd = 0;
	zlog_rule_t *a_rule;
	zlog_record_t *a_record;
	int i = 0;

	zc_assert(name, -1);
	zc_assert(record_output, -1);

	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag < 0) {
		zc_error("before use, must zlog_init first!!!");
		goto zlog_set_record_exit;
	}

	a_record = zlog_record_new(name, record_output);
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
