/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <errno.h>

#include "mdc.h"
#include "zc_defs.h"

/*******************************************************************************/

static xlog_mdc_kv_t *xlog_mdc_kv_new(char *key, char *value)
{
	xlog_mdc_kv_t *a_mdc_kv;
	size_t value_len;
	
	value_len = strlen(value);
	if(strlen(key) > MAXLEN_PATH || value_len > MAXLEN_PATH) {
		zc_error("key[%s] or value[%s] is too long", key, value);
		return NULL;
	}

	a_mdc_kv = calloc(1, sizeof(xlog_mdc_kv_t));
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

static void xlog_mdc_kv_del(xlog_mdc_kv_t *a_mdc_kv)
{
	zc_debug("del a_mdc_kv[%p], key[%s], value[%s]", a_mdc_kv, a_mdc_kv->key, a_mdc_kv->value);
	free(a_mdc_kv);
}

/*******************************************************************************/
xlog_mdc_t *xlog_mdc_new(void)
{
	int rc = 0;
	xlog_mdc_t *a_mdc;

	a_mdc = calloc(1, sizeof(xlog_mdc_t));
	if (!a_mdc) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_mdc->tab = zc_hashtable_new(20,
		zc_hashtable_str_hash, zc_hashtable_str_equal,
		NULL, (zc_hashtable_del_fn) xlog_mdc_kv_del);
	if (!a_mdc->tab) {
		zc_error("zc_hashtable_new fail");
		rc = -1;
		goto xlog_mdc_new_exit;
	}

      xlog_mdc_new_exit:
	if (rc) {
		xlog_mdc_del(a_mdc);
		return NULL;
	} else {
		return a_mdc;
	}
}

void xlog_mdc_del(xlog_mdc_t *a_mdc)
{
	zc_assert(a_mdc, );

	if (a_mdc->tab) {
		zc_hashtable_del(a_mdc->tab);
	}
	
	zc_debug("free a_mdc at[%p]", a_mdc);
	free(a_mdc);
	return;
}

void xlog_mdc_clean(xlog_mdc_t *a_mdc)
{
	zc_assert(a_mdc, );
	zc_hashtable_clean(a_mdc->tab);
	return;
}

int xlog_mdc_put(xlog_mdc_t *a_mdc, char *key, char *value)
{
	int rc = 0;
	xlog_mdc_kv_t *a_mdc_kv;

	zc_assert(a_mdc, -1);
	zc_assert(key, -1);
	zc_assert(value, -1);

	a_mdc_kv = xlog_mdc_kv_new(key, value);
	if (!a_mdc_kv) {
		zc_error("xlog_mdc_kv_new failed");
		return -1;
	}

	rc = zc_hashtable_put(a_mdc->tab, a_mdc_kv->key, a_mdc_kv);
	if (rc) {
		zc_error("zc_hashtable_put fail");
		xlog_mdc_kv_del(a_mdc_kv);
		return -1;
	}

	return 0;
}

char *xlog_mdc_get(xlog_mdc_t *a_mdc, char *key)
{
	xlog_mdc_kv_t *a_mdc_kv;

	zc_assert(a_mdc, NULL);
	zc_assert(key, NULL);

	a_mdc_kv = zc_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		zc_error("zc_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv->value;
	}
}

xlog_mdc_kv_t *xlog_mdc_get_kv(xlog_mdc_t *a_mdc, char *key)
{
	xlog_mdc_kv_t *a_mdc_kv;

	zc_assert(a_mdc, NULL);
	zc_assert(key, NULL);

	a_mdc_kv = zc_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		zc_error("zc_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv;
	}
}

void xlog_mdc_remove(xlog_mdc_t *a_mdc, char *key)
{
	zc_assert(a_mdc, );
	zc_assert(key, );

	zc_hashtable_remove(a_mdc->tab, key);
	return;
}
