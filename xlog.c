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
static pthread_rwlock_t xlog_env_lock = PTHREAD_RWLOCK_INITIALIZER;
static xlog_conf_t xlog_env_conf;
static xlog_tmap_t xlog_env_tmap;
static xlog_cmap_t xlog_env_cmap;
static int xlog_env_init_flag;
/*******************************************************************************/
int xlog_init(char *conf_file)
{
	int rc = 0;
	int rd = 0;

	zc_debug("------xlog_init start------");
	rd = pthread_rwlock_wrlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (xlog_env_init_flag > 0) {
		zc_error("already init, use xlog_update pls");
		rc = -1;
		goto xlog_init_exit;
	}

	rc = xlog_conf_init(&xlog_env_conf, conf_file);
	if (rc) {
		zc_error("conf_file[%s], init conf fail", conf_file);
		goto xlog_init_exit;
	}

	rc = xlog_rotater_init(&xlog_env_rotater, xlog_env_conf.rotate_lock_file);
	if (rc) {
		zc_error("xlog_rotater_init fail");
		goto xlog_init_exit;
	}

	rc = xlog_cmap_init(&xlog_env_cmap);
	if (rc) {
		zc_error("xlog_cmap_init fail");
		goto xlog_init_exit;
	}

	rc = xlog_tmap_init(&xlog_env_tmap);
	if (rc) {
		zc_error("xlog_tmap_init fail");
		goto xlog_init_exit;
	}

	xlog_env_init_flag = 1;

      xlog_init_exit:
	if (rc) {
		xlog_tmap_fini(&xlog_env_tmap);
		xlog_cmap_fini(&xlog_env_cmap);
		xlog_rotater_fini(&xlog_env_rotater);
		xlog_conf_fini(&xlog_env_conf);
		xlog_env_init_flag = -1;
	}

	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	zc_debug("------xlog_init end------ , rc[%d]", rc);
	return rc;
}

int xlog_update(char *conf_file)
{
	int rc = 0;
	int rd = 0;

	zc_debug("------xlog_update start------");
	rd = pthread_rwlock_wrlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("not init, use xlog_update pls");
		rc = -1;
		goto xlog_update_exit;
	}

	rc = xlog_conf_update(&xlog_env_conf, conf_file);
	if (rc) {
		zc_error("update from conf file[%s] fail", conf_file);
		goto xlog_update_exit;
	}

	rc = xlog_rotater_update(&xlog_env_rotater, xlog_env_conf.rotate_lock_file);
	if (rc) {
		zc_error("xlog_rotater_update fail");
		goto xlog_update_exit;
	}

	rc = xlog_cmap_update(&xlog_env_cmap, xlog_env_conf.rules);
	if (rc) {
		zc_error("xlog_cmap_update fail");
		goto xlog_update_exit;
	}

	rc = xlog_tmap_update(&xlog_env_tmap, xlog_env_conf.buf_size_min, xlog_env_conf.buf_size_max);
	if (rc) {
		zc_error("xlog_tmap_update fail");
		goto xlog_update_exit;
	}

	xlog_env_init_flag++;

      xlog_update_exit:
	if (rc) {
		xlog_tmap_fini(&xlog_env_tmap);
		xlog_cmap_fini(&xlog_env_cmap);
		xlog_rotater_fini(&xlog_env_rotater);
		xlog_conf_fini(&xlog_env_conf);
		xlog_env_init_flag = -1;
	}

	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	zc_debug("------xlog_update end------ , rc[%d]", rc);
	return rc;
}

void xlog_fini(void)
{
	int rd = 0;

	zc_debug("------xlog_fini start------");
	rd = pthread_rwlock_wrlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("before finish, must xlog_init fisrt");
		goto xlog_fini_exit;
	}

	xlog_tmap_fini(&xlog_env_tmap);
	xlog_cmap_fini(&xlog_env_cmap);
	xlog_rotater_fini(&xlog_env_rotater);
	xlog_conf_fini(&xlog_env_conf);
	xlog_env_init_flag = 0;

      xlog_fini_exit:
	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}

	zc_debug("------xlog_fini end------");
	return;
}

/*******************************************************************************/
xlog_category_t *xlog_get_category(char *category_name)
{
	int rd = 0;
	xlog_category_t *a_cat = NULL;

	zc_debug("------xlog_get_category[%s] start------", category_name);
	rd = pthread_rwlock_wrlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return NULL;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("before use, must xlog_init first!!!");
		a_cat = NULL;
		goto xlog_get_category_exit;
	}

	a_cat = xlog_cmap_fetch_category(&xlog_env_cmap, category_name, xlog_env_conf.rules);
	if (!a_cat) {
		zc_error("xlog_cmap_fetch_category fail");
		goto xlog_get_category_exit;
	}

      xlog_get_category_exit:
	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return NULL;
	}
	zc_debug("------xlog_get_category end------ ");
	return a_cat;
}

/*******************************************************************************/
int xlog_put_mdc(char *key, char *value)
{
	int rc = 0;
	int rd = 0;
	xlog_thread_t * a_thread;

	rd = pthread_rwlock_rdlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("before use, must xlog_init first!!!");
		rc = -1;
		goto xlog_put_mdc_exit;
	}

	a_thread = xlog_tmap_get_thread(&xlog_env_tmap);
	if (!a_thread) {

		rd = pthread_rwlock_unlock(&xlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto xlog_put_mdc_exit;
		}

		/* here between two lock, other tmap may create a thread */

		rd = pthread_rwlock_wrlock(&xlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto xlog_put_mdc_exit;
		}

		/* change to wrlock, try to get thread first
		 * to avoid oth thread make thread(buf&event) already
		 */
		a_thread = xlog_tmap_get_thread(&xlog_env_tmap);
		if (!a_thread) {
			zc_debug("in lock get thread again");
			a_thread = xlog_tmap_new_thread(&xlog_env_tmap,
							xlog_env_conf.buf_size_min, xlog_env_conf.buf_size_max);
			if (!a_thread) {
				zc_error("xlog_tmap_new_thread fail");
				rc = -1;
				goto xlog_put_mdc_exit;
			}
		}
	}

	rc = xlog_mdc_put(a_thread->mdc, key, value);
	if (rc) {
		zc_error("xlog_mdc_put fail, key[%s], value[%s]", key, value);
		goto xlog_put_mdc_exit;
	}

      xlog_put_mdc_exit:
	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	return rc;
}

char *xlog_get_mdc(char *key)
{
	int rd = 0;
	char *value = NULL;
	xlog_thread_t * a_thread;

	rd = pthread_rwlock_rdlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return NULL;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("before use, must xlog_init first!!!");
		goto xlog_get_mdc_exit;
	}

	a_thread = xlog_tmap_get_thread(&xlog_env_tmap);
	if (!a_thread) {
		zc_error("thread not new now, maybe not use xlog_put_mdc before");
		goto xlog_get_mdc_exit;
	}

	value = xlog_mdc_get(a_thread->mdc, key);
	if (!value) {
		zc_error("key not found in mdc, maybe not use xlog_put_mdc before");
		goto xlog_get_mdc_exit;
	}

      xlog_get_mdc_exit:
	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return NULL;
	}
	return value;
}

void xlog_remove_mdc(char *key)
{
	int rd = 0;
	xlog_thread_t * a_thread;

	rd = pthread_rwlock_rdlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("before use, must xlog_init first!!!");
		goto xlog_remove_mdc_exit;
	}

	a_thread = xlog_tmap_get_thread(&xlog_env_tmap);
	if (!a_thread) {
		zc_error("thread not new now, maybe not use xlog_put_mdc before");
		goto xlog_remove_mdc_exit;
	}

	xlog_mdc_remove(a_thread->mdc, key);

      xlog_remove_mdc_exit:
	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}

void xlog_clean_mdc(void)
{
	int rd = 0;
	xlog_thread_t * a_thread;

	rd = pthread_rwlock_rdlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_rdlock fail, rd[%d]", rd);
		return;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("before use, must xlog_init first!!!");
		goto xlog_clean_mdc_exit;
	}

	a_thread = xlog_tmap_get_thread(&xlog_env_tmap);
	if (!a_thread) {
		zc_error("thread not new now, maybe not use xlog_put_mdc before");
		goto xlog_clean_mdc_exit;
	}

	xlog_mdc_clean(a_thread->mdc);

      xlog_clean_mdc_exit:
	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}

/*******************************************************************************/

static int xlog_output(xlog_category_t *a_cat, char *file, long line,
		int priority, char *hex_buf, long hex_buf_len,
		char *str_format, va_list str_args, int generate_cmd)
{
	int rc = 0;
	int rd = 0;
	xlog_thread_t * a_thread;

	rd = pthread_rwlock_rdlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return -1;
	}

	if (xlog_env_init_flag <= 0) {
		zc_error("before use, must xlog_init first!!!");
		rc = -1;
		goto xlog_output_exit;
	}

	a_thread = xlog_tmap_get_thread(&xlog_env_tmap);
	if (!a_thread) {

		rd = pthread_rwlock_unlock(&xlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto xlog_output_exit;
		}

		/* here between two lock, other tmap may create a thread */

		rd = pthread_rwlock_wrlock(&xlog_env_lock);
		if (rd) {
			zc_error("pthread_rwlock_unlock fail, rd[%d]", rd);
			rc = -1;
			goto xlog_output_exit;
		}

		/* change to wrlock, try to get thread first
		 * to avoid oth thread make thread(buf&event) already
		 */
		a_thread = xlog_tmap_get_thread(&xlog_env_tmap);
		if (!a_thread) {
			zc_debug("in lock get thread again");
			a_thread = xlog_tmap_new_thread(&xlog_env_tmap,
							xlog_env_conf.buf_size_min, xlog_env_conf.buf_size_max);
			if (!a_thread) {
				zc_error("xlog_tmap_new_thread fail");
				rc = -1;
				goto xlog_output_exit;
			}
		}
	}

	xlog_event_set(a_thread->event,
				a_cat->name, &(a_cat->name_len), 
				file, line, priority,
				hex_buf, hex_buf_len, str_format, str_args, generate_cmd);

	rc = xlog_category_output(a_cat, a_thread);
	if (rc) {
		zc_error("xlog_output fail, srcfile[%s], srcline[%ld]", file, line);
		goto xlog_output_exit;
	}

      xlog_output_exit:
	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return -1;
	}

	return rc;
}
/*******************************************************************************/
void xlog(xlog_category_t *a_cat, char *file, long line, int priority, char *format, ...)
{
	va_list args;
	va_start(args, format);
	xlog_output(a_cat, file, line, priority, NULL, 0, format, args, XLOG_FMT);
	va_end(args);
}

void vxlog(xlog_category_t *a_cat, char *file, long line, int priority, char *format, va_list args)
{
	xlog_output(a_cat, file, line, priority, NULL, 0, format, args, XLOG_FMT);
}

void hxlog(xlog_category_t *a_cat, char *file, long line, int priority, char *buf, unsigned long buf_len)
{
	xlog_output(a_cat, file, line, priority, buf, buf_len, NULL, 0, XLOG_HEX);
}
/*******************************************************************************/
void xlog_profile(void)
{
	int rd = 0;
	rd = pthread_rwlock_rdlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_wrlock fail, rd[%d]", rd);
		return;
	}

	zc_error("------xlog_profile start------ ");
	zc_error("init_flag:[%d]", xlog_env_init_flag);
	xlog_conf_profile(&xlog_env_conf);
	xlog_tmap_profile(&xlog_env_tmap);
	xlog_cmap_profile(&xlog_env_cmap);
	zc_error("------xlog_profile end------ ");

	rd = pthread_rwlock_unlock(&xlog_env_lock);
	if (rd) {
		zc_error("pthread_rwlock_unlock fail, rd=[%d]", rd);
		return;
	}
	return;
}
