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

#include "conf.h"
#include "category_table.h"
#include "thread_table.h"
#include "mdc.h"
#include "zc_defs.h"
#include "rule.h"

/*******************************************************************************/
static pthread_rwlock_t zlog_env_lock = PTHREAD_RWLOCK_INITIALIZER;
static zlog_conf_t *zlog_env_conf;
static zc_hashtable_t *zlog_env_threads;
static zc_hashtable_t *zlog_env_categories;
static zlog_category_t *zlog_default_category;
static int zlog_env_init_flag = 0;
/*******************************************************************************/
/* inner no need thread-safe */
static void zlog_fini_inner(void)
{
	if (zlog_env_threads) zc_hashtable_del(zlog_env_threads);
	if (zlog_env_categories) zc_hashtable_del(zlog_env_categories);
	if (zlog_env_conf) zlog_conf_del(zlog_env_conf);
	zlog_env_threads = NULL;
	zlog_env_categories = NULL;
	zlog_env_conf = NULL;
	zlog_default_category = NULL;
	return;
}

static int zlog_init_inner(char *confpath)
{
	int rc = 0;
	zlog_thread_t *a_thread;
	size_t buf_size_min;
	size_t buf_size_max;

	zlog_env_conf = zlog_conf_new(confpath);
	if (!zlog_env_conf) {
		zc_error("zlog_conf_new[%s] fail", confpath);
		rc = -1;
		goto zlog_init_inner_exit;
	}

	zlog_env_categories = zlog_category_table_new();
	if (!zlog_env_categories) {
		zc_error("zlog_category_table_new fail");
		rc = -1;
		goto zlog_init_inner_exit;
	}

	zlog_env_threads = zlog_thread_table_new();
	if (!zlog_env_threads) {
		zc_error("zlog_thread_table_new fail");
		rc = -1;
		goto zlog_init_inner_exit;
	}

	zlog_conf_get_buf_size(zlog_env_conf, &buf_size_min, &buf_size_max);

	a_thread = zlog_thread_table_new_thread(zlog_env_threads,
				buf_size_min, buf_size_max);
	if (!a_thread) {
		zc_error("zlog_thread_table_new_thread fail");
		rc = -1;
		goto zlog_init_inner_exit;
	}

      zlog_init_inner_exit:
	if (rc) {
		zlog_fini_inner();
		return -1;
	} else {
		return 0;
	}
}

/*******************************************************************************/
int zlog_init(char *confpath)
{
	int rc = 0;
	int rd = 0;

	zc_debug("------zlog_init start, compile time[%s %s]------", __DATE__, __TIME__);

	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag > 0) {
		zc_error("already init, use zlog_reload pls");
		rc = -1;
		goto zlog_init_exit;
	}

	rc = zlog_init_inner(confpath);
	if (rc) {
		zc_error("zlog_init_inner[%s] fail", confpath);
		goto zlog_init_exit;
	}

	zlog_env_init_flag = 1;

      zlog_init_exit:
	zc_debug("------zlog_init end------ , rc[%d]", rc);
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}
	return rc;
}

int dzlog_init(char *confpath, char *cname)
{
	int rc = 0;
	int rd = 0;

	zc_debug("------dzlog_init start, compile time[%s %s]------", __DATE__, __TIME__);
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag > 0) {
		zc_error("already init, use zlog_reload pls");
		rc = -1;
		goto dzlog_init_exit;
	}

	rc = zlog_init_inner(confpath);
	if (rc) {
		zc_error("zlog_init_inner[%s] fail", confpath);
		goto dzlog_init_exit;
	}

	zlog_default_category = zlog_category_table_fetch_category(
				zlog_env_categories,
				cname,
				zlog_conf_get_rules(zlog_env_conf));
	if (!zlog_default_category) {
		zc_error("zlog_category_table_fetch_category[%s] fail",
				cname);
		rc = -1;
		goto dzlog_init_exit;
	}

	zlog_env_init_flag = 1;

      dzlog_init_exit:
	if (rc) zlog_fini_inner();
	zc_debug("------dzlog_init end------ , rc[%d]", rc);
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	return rc;
}
/*******************************************************************************/
int zlog_reload(char *confpath)
{
	int rc = 0;
	int rd = 0;
	zlog_conf_t *new_conf = NULL;
	size_t new_buf_size_min;
	size_t new_buf_size_max;
	size_t old_buf_size_min;
	size_t old_buf_size_max;

	zc_debug("------zlog_reload start------");
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("never zlog_init before");
		rc = -1;
		goto zlog_reload_exit_2;
	}

	if (confpath == NULL) {
		/* use last conf file */
		confpath = zlog_conf_get_file(zlog_env_conf);
	}

	new_conf = zlog_conf_new(confpath);
	if (!new_conf) {
		zc_error("zlog_conf_new fail");
		rc = -1;
		goto zlog_reload_exit;
	}

	rc = zlog_category_table_update_rules(zlog_env_categories,
			 	zlog_conf_get_rules(new_conf));
	if (rc) {
		zc_error("zlog_category_table_update fail");
		rc = -1;
		goto zlog_reload_exit;
	}

	zlog_conf_get_buf_size(zlog_env_conf, &old_buf_size_min, &old_buf_size_max);
	zlog_conf_get_buf_size(new_conf, &new_buf_size_min, &new_buf_size_max);

	if ((new_buf_size_min != old_buf_size_min)
		|| (new_buf_size_max != old_buf_size_max) ) {
		rc = zlog_thread_table_update_msg_buf(zlog_env_threads,
				new_buf_size_min, new_buf_size_max);
		if (rc) {
			zc_error("zlog_category_table_update fail");
			rc = -1;
			goto zlog_reload_exit;
		}
	}

	zlog_env_init_flag++;

      zlog_reload_exit:
	if (rc) {
		/* roll back everything */
		zc_warn("zlog_reload fail, use old conf file, still working");
		if (new_conf) zlog_conf_del(new_conf);
		zlog_category_table_rollback_rules(zlog_env_categories);
		zlog_thread_table_rollback_msg_buf(zlog_env_threads);
	} else {
		zlog_category_table_commit_rules(zlog_env_categories);
		zlog_thread_table_commit_msg_buf(zlog_env_threads);
		zlog_conf_del(zlog_env_conf);
		zlog_env_conf = new_conf;
	}

	zc_debug("------zlog_reload [%d] times end------ , rc[%d]", zlog_env_init_flag, rc);
      zlog_reload_exit_2:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}
	return rc;
}
/*******************************************************************************/
void zlog_fini(void)
{
	int rd = 0;

	zc_debug("------zlog_fini start------");
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before finish, must zlog_init fisrt");
		goto zlog_fini_exit;
	}

	zlog_fini_inner();
	zlog_env_init_flag = 0;

      zlog_fini_exit:
	zc_debug("------zlog_fini end------");
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}
/*******************************************************************************/
zlog_category_t *zlog_get_category(char *cname)
{
	int rd = 0;
	zlog_category_t *a_category = NULL;

	zc_assert(cname, NULL);
	zc_debug("------zlog_get_category[%s] start------", cname);
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return NULL;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		a_category = NULL;
		goto zlog_get_category_exit;
	}

	a_category = zlog_category_table_fetch_category(
				zlog_env_categories,
				cname,
				zlog_conf_get_rules(zlog_env_conf));
	if (!a_category) {
		zc_error("zlog_category_table_fetch_category[%s] fail", cname);
		goto zlog_get_category_exit;
	}

      zlog_get_category_exit:
	zc_debug("------zlog_get_category[%s] end------ ", cname);
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return NULL;
	}
	return a_category;
}

int dzlog_set_category(char *cname)
{
	int rd = 0;
	int rc = 0;

	zc_assert(cname, -1);

	zc_debug("------dzlog_set_category[%s] start------", cname);
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		rc = -1;
		goto dzlog_set_category_exit;
	}

	zlog_default_category = zlog_category_table_fetch_category(
				zlog_env_categories,
				cname,
				zlog_conf_get_rules(zlog_env_conf));
	if (!zlog_default_category) {
		zc_error("zlog_category_table_fetch_category[%s] fail", cname);
		rc = -1;
		goto dzlog_set_category_exit;
	}

      dzlog_set_category_exit:
	zc_debug("------dzlog_set_category[%s] end, rc[%d]------ ", cname, rc);
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}
	return rc;
}
/*******************************************************************************/
int zlog_put_mdc(char *key, char *value)
{
	int rc = 0;
	int rd = 0;
	zlog_thread_t *a_thread;

	zc_assert(key, -1);
	zc_assert(value, -1);

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		rc = -1;
		goto zlog_put_mdc_exit;
	}

	a_thread = zlog_thread_table_get_thread(zlog_env_threads, pthread_self());
	if (!a_thread) {
		rd = pthread_rwlock_unlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_put_mdc_exit;
		}
		/* here between two lock, other thread_table may create a thread */
		rd = pthread_rwlock_wrlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_put_mdc_exit;
		}
		/* change to wrlock, try to get thread first
		 * to avoid oth thread make thread(buf&event) already
		 */
		a_thread = zlog_thread_table_get_thread(zlog_env_threads, pthread_self());
		if (!a_thread) {
			size_t buf_size_min;
			size_t buf_size_max;
			zlog_conf_get_buf_size(zlog_env_conf, &buf_size_min, &buf_size_max);
			a_thread = zlog_thread_table_new_thread(zlog_env_threads,
							buf_size_min, buf_size_max);
			if (!a_thread) {
				zc_error("zlog_conf_get_buf_size fail");
				rc = -1;
				goto zlog_put_mdc_exit;
			}
		}
	}

	rc = zlog_mdc_put(a_thread->mdc, key, value);
	if (rc) {
		zc_error("zlog_mdc_put fail, key[%s], value[%s]", key, value);
		goto zlog_put_mdc_exit;
	}

      zlog_put_mdc_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	return rc;
}

char *zlog_get_mdc(char *key)
{
	int rd = 0;
	char *value = NULL;
	zlog_thread_t *a_thread;

	zc_assert(key, NULL);

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return NULL;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		goto zlog_get_mdc_exit;
	}

	a_thread = zlog_thread_table_get_thread(zlog_env_threads, pthread_self());
	if (!a_thread) {
		zc_error("thread not found, maybe not use zlog_put_mdc before");
		goto zlog_get_mdc_exit;
	}

	value = zlog_mdc_get(a_thread->mdc, key);
	if (!value) {
		zc_error("key[%s] not found in mdc", key);
		goto zlog_get_mdc_exit;
	}

      zlog_get_mdc_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return NULL;
	}
	return value;
}

void zlog_remove_mdc(char *key)
{
	int rd = 0;
	zlog_thread_t *a_thread;

	zc_assert(key, );

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		goto zlog_remove_mdc_exit;
	}

	a_thread = zlog_thread_table_get_thread(zlog_env_threads, pthread_self());
	if (!a_thread) {
		zc_error("thread not new now, maybe not use zlog_put_mdc before");
		goto zlog_remove_mdc_exit;
	}

	zlog_mdc_remove(a_thread->mdc, key);

      zlog_remove_mdc_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}

void zlog_clean_mdc(void)
{
	int rd = 0;
	zlog_thread_t *a_thread;

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		goto zlog_clean_mdc_exit;
	}

	a_thread = zlog_thread_table_get_thread(zlog_env_threads, pthread_self());
	if (!a_thread) {
		zc_error("thread not new now, maybe not use zlog_put_mdc before");
		goto zlog_clean_mdc_exit;
	}

	zlog_mdc_clean(a_thread->mdc);

      zlog_clean_mdc_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}

/*******************************************************************************/

static int zlog_output(zlog_category_t * a_category,
		const char *file, size_t file_len, const char *func, size_t func_len, long line, int level,
		const void *hex_buf, size_t hex_buf_len, const char *str_format, va_list str_args,
		int generate_cmd)
{
	int rc = 0;
	int rd = 0;
	zlog_thread_t *a_thread;

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		rc = -1;
		goto zlog_output_exit;
	}

	a_thread = zlog_thread_table_get_thread(zlog_env_threads, pthread_self());
	if (!a_thread) {
		rd = pthread_rwlock_unlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_output_exit;
		}
		/* here between two lock's gap, other thread_table maybe create a thread */
		rd = pthread_rwlock_wrlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_output_exit;
		}

		/* change to wrlock, try to get thread first
		 * to avoid oth thread make thread(buf&event) already
		 */
		a_thread = zlog_thread_table_get_thread(zlog_env_threads, pthread_self());
		if (!a_thread) {
			size_t buf_size_min;
			size_t buf_size_max;
			zlog_conf_get_buf_size(zlog_env_conf, &buf_size_min, &buf_size_max);
			a_thread = zlog_thread_table_new_thread(zlog_env_threads,
							buf_size_min, buf_size_max);
			if (!a_thread) {
				zc_error("zlog_thread_table_new_thread fail");
				rc = -1;
				goto zlog_output_exit;
			}
		}
	}

	zlog_event_set(a_thread->event,
			   a_category->name, a_category->name_len,
			   file, file_len, func, func_len, line, level,
			   hex_buf, hex_buf_len, str_format, str_args,
			   generate_cmd);

	rc = zlog_category_output(a_category, a_thread);
	if (rc) {
		zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file,
			 line);
		goto zlog_output_exit;
	}

      zlog_output_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	return rc;
}

/* for speed up, copy from zlog_output */
static int dzlog_output(const char *file, size_t file_len, const char *func, size_t func_len,
		long line, int level,
		const void *hex_buf, size_t hex_buf_len, const char *str_format, va_list str_args,
		int generate_cmd)
{
	int rc = 0;
	int rd = 0;
	zlog_thread_t *a_thread;
	pthread_t tid;

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		rc = -1;
		goto zlog_output_exit;
	}

	/* that's the differnce, must judge default_category in lock */
	if (!zlog_default_category) {
		zc_error("zlog_default_category is null,"
			"dzlog_init() or dzlog_set_cateogry() is not called above");
		rc = -1;
		goto zlog_output_exit;
	}

	tid = pthread_self();
	a_thread = zlog_thread_table_get_thread(zlog_env_threads, tid);
	if (!a_thread) {
		rd = pthread_rwlock_unlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_output_exit;
		}
		/* here between two lock's gap, other thread_table maybe create a thread */
		rd = pthread_rwlock_wrlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_output_exit;
		}

		/* change to wrlock, try to get thread first
		 * to avoid oth thread make thread(buf&event) already
		 */
		a_thread = zlog_thread_table_get_thread(zlog_env_threads, tid);
		if (!a_thread) {
			size_t buf_size_min;
			size_t buf_size_max;
			zlog_conf_get_buf_size(zlog_env_conf, &buf_size_min, &buf_size_max);
			a_thread = zlog_thread_table_new_thread(zlog_env_threads,
							buf_size_min, buf_size_max);
			if (!a_thread) {
				zc_error("zlog_thread_table_new_thread fail");
				rc = -1;
				goto zlog_output_exit;
			}
		}
	}

	zlog_event_set(a_thread->event,
			zlog_default_category->name, zlog_default_category->name_len,
			file, file_len, func, func_len, line, level,
			hex_buf, hex_buf_len, str_format, str_args,
			generate_cmd);


	rc = zlog_category_output(zlog_default_category, a_thread);
	if (rc) {
		zc_error("zlog_output fail, srcfile[%s], srcline[%ld]", file,
			 line);
		goto zlog_output_exit;
	}

      zlog_output_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	return rc;
}

/*******************************************************************************/
void zlog(zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen,
	long line, const int level,
	const char *format, ...)
{
	va_list args;
	va_start(args, format);
	zlog_output(category, file, filelen, func, funclen, line, level, NULL, 0, format, args, ZLOG_FMT);
	va_end(args);
}

void vzlog(zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const char *format, va_list args)
{
	zlog_output(category, file, filelen, func, funclen, line, level, NULL, 0, format, args, ZLOG_FMT);
}

void hzlog(zlog_category_t * category,
	const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const void *buf, size_t buf_len)
{
	zlog_output(category, file, filelen, func, funclen, line, level, buf, buf_len, NULL, 0, ZLOG_HEX);
}

/*******************************************************************************/
void dzlog(const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const char *format, ...)
{
	va_list args;
	va_start(args, format);
	dzlog_output(file, filelen, func, funclen, line, level, NULL, 0, format, args, ZLOG_FMT);
	va_end(args);
}

void vdzlog(const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const char *format, va_list args)
{
	dzlog_output(file, filelen, func, funclen, line, level, NULL, 0, format, args, ZLOG_FMT);
}

void hdzlog(const char *file, size_t filelen, const char *func, size_t funclen, long line, int level,
	const void *buf, size_t buf_len)
{
	dzlog_output(file, filelen, func, funclen, line, level, buf, buf_len, NULL, 0, ZLOG_HEX);
}

/*******************************************************************************/
void zlog_profile(void)
{
	int rd = 0;
	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return;
	}
	zc_warn("------zlog_profile start------ ");
	zc_warn("init_flag:[%d]", zlog_env_init_flag);
	zlog_conf_profile(zlog_env_conf, ZC_WARN);
	zlog_thread_table_profile(zlog_env_threads, ZC_WARN);
	zlog_category_table_profile(zlog_env_categories, ZC_WARN);
	if (zlog_default_category) {
		zc_warn("-default_category-");
		zlog_category_profile(zlog_default_category, ZC_WARN);
	}
	zc_warn("------zlog_profile end------ ");
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}
/*******************************************************************************/
int zlog_set_record(char *str1, zlog_record_fn record)
{
	int rd = 0;
	zlog_rule_t *a_rule;
	int i = 0;

	zc_assert(str1, -1);

	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		goto zlog_set_record_exit;
	}

	zc_arraylist_foreach(zlog_conf_get_rules(zlog_env_conf), i, a_rule) {
		zlog_rule_set_record(a_rule, str1, record);
	}

      zlog_set_record_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}
	return 0;
}
