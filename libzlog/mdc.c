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

#include <stdlib.h>
#include <errno.h>

#include "mdc.h"
#include "zc_defs.h"

/*******************************************************************************/

static zlog_mdc_kv_t *zlog_mdc_kv_new(char *key, char *value)
{
	zlog_mdc_kv_t *a_mdc_kv;
	size_t value_len;

	value_len = strlen(value);
	if (strlen(key) > MAXLEN_PATH || value_len > MAXLEN_PATH) {
		zc_error("key[%s] or value[%s] is too long", key, value);
		return NULL;
	}

	a_mdc_kv = calloc(1, sizeof(zlog_mdc_kv_t));
	if (!a_mdc_kv) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	strcpy(a_mdc_kv->key, key);
	strcpy(a_mdc_kv->value, value);
	a_mdc_kv->value_len = value_len;
	zc_debug("new a_mdc_kv[%p]", a_mdc_kv);
	return a_mdc_kv;
}

static void zlog_mdc_kv_del(zlog_mdc_kv_t * a_mdc_kv)
{
	zc_debug("del a_mdc_kv[%p], key[%s], value[%s]", a_mdc_kv,
		 a_mdc_kv->key, a_mdc_kv->value);
	free(a_mdc_kv);
}

/*******************************************************************************/
zlog_mdc_t *zlog_mdc_new(void)
{
	int rc = 0;
	zlog_mdc_t *a_mdc;

	a_mdc = calloc(1, sizeof(zlog_mdc_t));
	if (!a_mdc) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_mdc->tab = zc_hashtable_new(20,
				      zc_hashtable_str_hash,
				      zc_hashtable_str_equal, NULL,
				      (zc_hashtable_del_fn) zlog_mdc_kv_del);
	if (!a_mdc->tab) {
		zc_error("zc_hashtable_new fail");
		rc = -1;
		goto zlog_mdc_new_exit;
	}

      zlog_mdc_new_exit:
	if (rc) {
		zlog_mdc_del(a_mdc);
		return NULL;
	} else {
		return a_mdc;
	}
}

void zlog_mdc_del(zlog_mdc_t * a_mdc)
{
	zc_assert_debug(a_mdc,);

	if (a_mdc->tab) {
		zc_hashtable_del(a_mdc->tab);
	}

	zc_debug("free a_mdc at[%p]", a_mdc);
	free(a_mdc);
	return;
}

void zlog_mdc_clean(zlog_mdc_t * a_mdc)
{
	zc_assert_debug(a_mdc,);
	zc_hashtable_clean(a_mdc->tab);
	return;
}

int zlog_mdc_put(zlog_mdc_t * a_mdc, char *key, char *value)
{
	int rc = 0;
	zlog_mdc_kv_t *a_mdc_kv;

	zc_assert_debug(a_mdc, -1);
	zc_assert_debug(key, -1);
	zc_assert_debug(value, -1);

	a_mdc_kv = zlog_mdc_kv_new(key, value);
	if (!a_mdc_kv) {
		zc_error("zlog_mdc_kv_new failed");
		return -1;
	}

	rc = zc_hashtable_put(a_mdc->tab, a_mdc_kv->key, a_mdc_kv);
	if (rc) {
		zc_error("zc_hashtable_put fail");
		zlog_mdc_kv_del(a_mdc_kv);
		return -1;
	}

	return 0;
}

char *zlog_mdc_get(zlog_mdc_t * a_mdc, char *key)
{
	zlog_mdc_kv_t *a_mdc_kv;

	zc_assert_debug(a_mdc, NULL);
	zc_assert_debug(key, NULL);

	a_mdc_kv = zc_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		zc_error("zc_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv->value;
	}
}

zlog_mdc_kv_t *zlog_mdc_get_kv(zlog_mdc_t * a_mdc, char *key)
{
	zlog_mdc_kv_t *a_mdc_kv;

	zc_assert_debug(a_mdc, NULL);
	zc_assert_debug(key, NULL);

	a_mdc_kv = zc_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		zc_error("zc_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv;
	}
}

void zlog_mdc_remove(zlog_mdc_t * a_mdc, char *key)
{
	zc_assert_debug(a_mdc,);
	zc_assert_debug(key,);

	zc_hashtable_remove(a_mdc->tab, key);
	return;
}
