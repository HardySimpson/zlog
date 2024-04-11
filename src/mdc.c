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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "mdc.h"
#include "zc_defs.h"

void zlog_mdc_profile(zlog_mdc_t *a_mdc, int flag)
{
	zc_hashtable_entry_t *a_entry;
	zlog_mdc_kv_t *a_mdc_kv;

	zc_assert(a_mdc,);
	zc_profile(flag, "---mdc[%p]---", a_mdc);

	zc_hashtable_foreach(a_mdc->tab, a_entry) {
		a_mdc_kv = a_entry->value;
		zc_profile(flag, "----mdc_kv[%p][%s]-[%s]----",
				a_mdc_kv,
				a_mdc_kv->key, a_mdc_kv->value);
	}
	return;
}
/*******************************************************************************/
void zlog_mdc_del(zlog_mdc_t * a_mdc)
{
	zc_assert(a_mdc,);
	if (a_mdc->tab) zc_hashtable_del(a_mdc->tab);
	zc_debug("zlog_mdc_del[%p]", a_mdc);
    free(a_mdc);
	return;
}

static void zlog_mdc_kv_del(zlog_mdc_kv_t * a_mdc_kv)
{
	zc_debug("zlog_mdc_kv_del[%p]", a_mdc_kv);
    free(a_mdc_kv);
}

static zlog_mdc_kv_t *zlog_mdc_kv_new(const char *key, const char *value)
{
	zlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = calloc(1, sizeof(zlog_mdc_kv_t));
	if (!a_mdc_kv) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	snprintf(a_mdc_kv->key, sizeof(a_mdc_kv->key), "%s", key);
	a_mdc_kv->value_len = snprintf(a_mdc_kv->value, sizeof(a_mdc_kv->value), "%s", value);
	return a_mdc_kv;
}

zlog_mdc_t *zlog_mdc_new(void)
{
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
		goto err;
	}

	//zlog_mdc_profile(a_mdc, ZC_DEBUG);
	return a_mdc;
err:
	zlog_mdc_del(a_mdc);
	return NULL;
}

/*******************************************************************************/
int zlog_mdc_put(zlog_mdc_t * a_mdc, const char *key, const char *value)
{
	zlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = zlog_mdc_kv_new(key, value);
	if (!a_mdc_kv) {
		zc_error("zlog_mdc_kv_new failed");
		return -1;
	}

	if (zc_hashtable_put(a_mdc->tab, a_mdc_kv->key, a_mdc_kv)) {
		zc_error("zc_hashtable_put fail");
		zlog_mdc_kv_del(a_mdc_kv);
		return -1;
	}

	return 0;
}

void zlog_mdc_clean(zlog_mdc_t * a_mdc)
{
	zc_hashtable_clean(a_mdc->tab);
	return;
}

char *zlog_mdc_get(zlog_mdc_t * a_mdc, const char *key)
{
	zlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = zc_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		zc_error("zc_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv->value;
	}
}

zlog_mdc_kv_t *zlog_mdc_get_kv(zlog_mdc_t * a_mdc, const char *key)
{
	zlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = zc_hashtable_get(a_mdc->tab, key);
	if (!a_mdc_kv) {
		zc_error("zc_hashtable_get fail");
		return NULL;
	} else {
		return a_mdc_kv;
	}
}

void zlog_mdc_remove(zlog_mdc_t * a_mdc, const char *key)
{
	zc_hashtable_remove(a_mdc->tab, key);
	return;
}
