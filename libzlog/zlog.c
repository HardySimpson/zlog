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
#include "rotater.h"
#include "category.h"
#include "thread.h"
#include "zc_defs.h"
#include "mdc.h"

/*******************************************************************************/
static pthread_rwlock_t zlog_env_lock = PTHREAD_RWLOCK_INITIALIZER;
static zlog_conf_t zlog_env_conf;
static zlog_tmap_t zlog_env_tmap;
static zlog_cmap_t zlog_env_cmap;
static int zlog_env_init_flag = 0;
/*******************************************************************************/
int zlog_init(char *conf_file)
{
	int rc = 0;
	int rd = 0;
	zlog_thread_t *a_thread;

	zc_debug("------zlog_init start, compile time[%s]------", __TIME__);
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag > 0) {
		zc_error("already init, use zlog_update pls");
		rc = -1;
		goto zlog_init_exit;
	}

	rc = zlog_conf_init(&zlog_env_conf, conf_file);
	if (rc) {
		zc_error("conf_file[%s], init conf fail", conf_file);
		goto zlog_init_exit;
	}

	rc = zlog_rotater_init(&zlog_env_rotater,
			       zlog_env_conf.rotate_lock_file);
	if (rc) {
		zc_error("zlog_rotater_init fail");
		goto zlog_init_exit;
	}

	rc = zlog_cmap_init(&zlog_env_cmap);
	if (rc) {
		zc_error("zlog_cmap_init fail");
		goto zlog_init_exit;
	}

	rc = zlog_tmap_init(&zlog_env_tmap);
	if (rc) {
		zc_error("zlog_tmap_init fail");
		goto zlog_init_exit;
	}

	zc_debug("new all data(buf,event...)  in zlog_init thread");
	a_thread = zlog_tmap_new_thread(
				&zlog_env_tmap,
				zlog_env_conf.buf_size_min,
				zlog_env_conf.buf_size_max
			);
	if (!a_thread) {
		zc_error("zlog_tmap_new_thread fail");
		rc = -1;
		goto zlog_init_exit;
	}

	zlog_env_init_flag = 1;

      zlog_init_exit:
	if (rc) {
		zlog_tmap_fini(&zlog_env_tmap);
		zlog_cmap_fini(&zlog_env_cmap);
		zlog_rotater_fini(&zlog_env_rotater);
		zlog_conf_fini(&zlog_env_conf);
		zlog_env_init_flag = -1;
	}

	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	zc_debug("------zlog_init end------ , rc[%d]", rc);
	return rc;
}

int zlog_update(char *conf_file)
{
	int rc = 0;
	int rd = 0;

	zc_debug("------zlog_update start------");
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("not init, use zlog_update pls");
		rc = -1;
		goto zlog_update_exit;
	}

	rc = zlog_conf_update(&zlog_env_conf, conf_file);
	if (rc) {
		zc_error("update from conf file[%s] fail", conf_file);
		goto zlog_update_exit;
	}

	rc = zlog_rotater_update(&zlog_env_rotater,
				 zlog_env_conf.rotate_lock_file);
	if (rc) {
		zc_error("zlog_rotater_update fail");
		goto zlog_update_exit;
	}

	rc = zlog_cmap_update(&zlog_env_cmap, zlog_env_conf.rules);
	if (rc) {
		zc_error("zlog_cmap_update fail");
		goto zlog_update_exit;
	}

	rc = zlog_tmap_update(&zlog_env_tmap, zlog_env_conf.buf_size_min,
			      zlog_env_conf.buf_size_max);
	if (rc) {
		zc_error("zlog_tmap_update fail");
		goto zlog_update_exit;
	}

	zlog_env_init_flag++;

      zlog_update_exit:
	if (rc) {
		zlog_tmap_fini(&zlog_env_tmap);
		zlog_cmap_fini(&zlog_env_cmap);
		zlog_rotater_fini(&zlog_env_rotater);
		zlog_conf_fini(&zlog_env_conf);
		zlog_env_init_flag = -1;
	}

	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	zc_debug("------zlog_update end------ , rc[%d]", rc);
	return rc;
}

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

	zlog_tmap_fini(&zlog_env_tmap);
	zlog_cmap_fini(&zlog_env_cmap);
	zlog_rotater_fini(&zlog_env_rotater);
	zlog_conf_fini(&zlog_env_conf);
	zlog_env_init_flag = 0;

      zlog_fini_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}

	zc_debug("------zlog_fini end------");
	return;
}

/*******************************************************************************/
zlog_category_t *zlog_get_category(char *category_name)
{
	int rd = 0;
	zlog_category_t *a_cat = NULL;

	zc_assert_runtime(category_name, NULL);

	zc_debug("------zlog_get_category[%s] start------", category_name);
	rd = pthread_rwlock_wrlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return NULL;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		a_cat = NULL;
		goto zlog_get_category_exit;
	}

	a_cat =
	    zlog_cmap_fetch_category(&zlog_env_cmap, category_name,
				     zlog_env_conf.rules);
	if (!a_cat) {
		zc_error("zlog_cmap_fetch_category fail");
		goto zlog_get_category_exit;
	}

      zlog_get_category_exit:
	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return NULL;
	}
	zc_debug("------zlog_get_category end------ ");
	return a_cat;
}

/*******************************************************************************/
int zlog_put_mdc(char *key, char *value)
{
	int rc = 0;
	int rd = 0;
	zlog_thread_t *a_thread;

	zc_assert_runtime(key, -1);
	zc_assert_runtime(value, -1);

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

	a_thread = zlog_tmap_get_thread(&zlog_env_tmap);
	if (!a_thread) {

		rd = pthread_rwlock_unlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_put_mdc_exit;
		}

		/* here between two lock, other tmap may create a thread */

		rd = pthread_rwlock_wrlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_put_mdc_exit;
		}

		/* change to wrlock, try to get thread first
		 * to avoid oth thread make thread(buf&event) already
		 */
		a_thread = zlog_tmap_get_thread(&zlog_env_tmap);
		if (!a_thread) {
			zc_debug("in lock get thread again");
			a_thread = zlog_tmap_new_thread(&zlog_env_tmap,
							zlog_env_conf.
							buf_size_min,
							zlog_env_conf.
							buf_size_max);
			if (!a_thread) {
				zc_error("zlog_tmap_new_thread fail");
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

	zc_assert_runtime(key, NULL);

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return NULL;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		goto zlog_get_mdc_exit;
	}

	a_thread = zlog_tmap_get_thread(&zlog_env_tmap);
	if (!a_thread) {
		zc_error
		    ("thread not new now, maybe not use zlog_put_mdc before");
		goto zlog_get_mdc_exit;
	}

	value = zlog_mdc_get(a_thread->mdc, key);
	if (!value) {
		zc_error
		    ("key not found in mdc, maybe not use zlog_put_mdc before");
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

	zc_assert_runtime(key, );

	rd = pthread_rwlock_rdlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return;
	}

	if (zlog_env_init_flag <= 0) {
		zc_error("before use, must zlog_init first!!!");
		goto zlog_remove_mdc_exit;
	}

	a_thread = zlog_tmap_get_thread(&zlog_env_tmap);
	if (!a_thread) {
		zc_error
		    ("thread not new now, maybe not use zlog_put_mdc before");
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

	a_thread = zlog_tmap_get_thread(&zlog_env_tmap);
	if (!a_thread) {
		zc_error
		    ("thread not new now, maybe not use zlog_put_mdc before");
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

static int zlog_output(zlog_category_t * a_cat, char *file, long line,
		       int priority, char *hex_buf, long hex_buf_len,
		       char *str_format, va_list str_args, int generate_cmd)
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

	a_thread = zlog_tmap_get_thread(&zlog_env_tmap);
	if (!a_thread) {

		rd = pthread_rwlock_unlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_output_exit;
		}

		/* here between two lock's gap, other tmap maybe create a thread */

		rd = pthread_rwlock_wrlock(&zlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto zlog_output_exit;
		}

		/* change to wrlock, try to get thread first
		 * to avoid oth thread make thread(buf&event) already
		 */
		a_thread = zlog_tmap_get_thread(&zlog_env_tmap);
		if (!a_thread) {
			zc_debug("in lock get thread again");
			a_thread = zlog_tmap_new_thread(&zlog_env_tmap,
							zlog_env_conf.
							buf_size_min,
							zlog_env_conf.
							buf_size_max);
			if (!a_thread) {
				zc_error("zlog_tmap_new_thread fail");
				rc = -1;
				goto zlog_output_exit;
			}
		}
	}

	zlog_event_refresh(a_thread->event,
			   a_cat->name, &(a_cat->name_len),
			   file, line, priority,
			   hex_buf, hex_buf_len, str_format, str_args,
			   generate_cmd);

	rc = zlog_category_output(a_cat, a_thread);
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
void zlog(zlog_category_t * a_cat, char *file, long line, int priority,
	  char *format, ...)
{
	zc_assert_runtime(a_cat, );

	va_list args;
	va_start(args, format);
	zlog_output(a_cat, file, line, priority, NULL, 0, format, args,
		    ZLOG_FMT);
	va_end(args);
}

void vzlog(zlog_category_t * a_cat, char *file, long line, int priority,
	   char *format, va_list args)
{
	zc_assert_runtime(a_cat, );

	zlog_output(a_cat, file, line, priority, NULL, 0, format, args,
		    ZLOG_FMT);
}

void hzlog(zlog_category_t * a_cat, char *file, long line, int priority,
	   char *buf, unsigned long buf_len)
{
	zc_assert_runtime(a_cat, );

	zlog_output(a_cat, file, line, priority, buf, buf_len, NULL, 0,
		    ZLOG_HEX);
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

	zc_error("------zlog_profile start------ ");
	zc_error("init_flag:[%d]", zlog_env_init_flag);
	zlog_conf_profile(&zlog_env_conf);
	zlog_tmap_profile(&zlog_env_tmap);
	zlog_cmap_profile(&zlog_env_cmap);
	zc_error("------zlog_profile end------ ");

	rd = pthread_rwlock_unlock(&zlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}
