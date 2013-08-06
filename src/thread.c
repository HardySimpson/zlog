/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#include <pthread.h>
#include <errno.h>

#include "zc_defs.h"
#include "event.h"
#include "buf.h"
#include "thread.h"
#include "mdc.h"

void zlog_thread_profile(zlog_thread_t * a_thread, int flag)
{
	zc_assert(a_thread,);
	zc_profile(flag, "--thread[%p][%d][%p,%p,%p,%p,%p][%s]--",
			a_thread,
			a_thread->version,
			a_thread->conf,
			a_thread->mdc,
			a_thread->event,
			a_thread->categories
			a_thread->msg);

	zlog_conf_profile(a_thread->conf, flag);
	zlog_mdc_profile(a_thread->mdc, flag);
	zlog_event_profile(a_thread->event, flag);
	zlog_category_table_profile(a_thread->categories, flag);
	return;
}
/*******************************************************************************/
void zlog_thread_del(zlog_thread_t * a_thread)
{
	zc_assert(a_thread,);
	if (a_thread->conf) zlog_conf_del(a_thread->conf);
	if (a_thread->categories) zlog_hashtable_del(a_thread->categories);

	if (a_thread->mdc) zlog_mdc_del(a_thread->mdc);
	if (a_thread->event) zlog_event_del(a_thread->event);
	if (a_thread->msg) zc_sdsfree(a_thread->msg);

	free(a_thread);
	zc_debug("zlog_thread_del[%p]", a_thread);
	return;
}

zlog_thread_t *zlog_thread_new(zlog_conf_t *a_conf, int version)
{
	zlog_thread_t *a_thread;

	a_thread = calloc(1, sizeof(zlog_thread_t));
	if (!a_thread) { zc_error("calloc fail, errno[%d]", errno); return NULL; }

	a_thread->version = version;

	a_thread->conf = zlog_conf_dup(a_conf);
	if (!a_thread->conf) { zc_error("zlog_conf_dup fail"); goto err; }

	a_thread->categories = zc_hashtable_new(20, &zlog_category_hash_type);
	if (!a_thread->categories) { zc_error("zc_hashtable_new fail"); goto err; }

	a_thread->mdc = zlog_mdc_new();
	if (!a_thread->mdc) { zc_error("zlog_mdc_new fail"); goto err; }

	a_thread->event = zlog_event_new();
	if (!a_thread->event) { zc_error("zlog_event_new fail"); goto err; }

	a_thread->msg = zc_sdsnewlen(NULL, 512);
	if (a_thread->msg) { zc_error("zc_sdsnewlen fail"); goto err; }

	//zlog_thread_profile(a_thread, ZC_DEBUG);
	return a_thread;
err:
	zlog_thread_del(a_thread);
	return NULL;
}
/*******************************************************************************/
zlog_category_t *zlog_thread_fetch_category(zlog_thread_t * a_thread, const char *cname)
{
	int rc;
	zlog_category_t *a_category;

	a_category = zc_hastable_get(cname);
	if (a_category) return category; /* else never used before */

	a_category = zlog_category_new(cname, a_thread->version,
			a_thread->event, a_thread->mdc, a_thread->conf->rules);
	if (!category) { zc_error("zlog_category_new fail"); return NULL; }

	rc = zc_hashtable_put(a_thread->categories, a_category->name, a_category);
	if (rc) { zlog_category_del(a_category); zc_error("zc_hashtable_put fail"); return NULL; } 

	return a_category;
}

/*******************************************************************************/
int zlog_thread_update(zlog_thread_t *a_thread, zlog_conf_t * a_conf, int version)
{
	int rc;
	zlog_conf_t *b_conf = NULL;
	zc_hashtable_t *b_categories = NULL;
	zlog_category_t *b_category = NULL;
	zlog_category_t *a_category = NULL;

	b_conf = zlog_conf_dup(a_conf);
	if (!b_conf) { zc_error("zlog_conf_dup fail"); goto err; }

	b_categories = zc_hashtable_new(20, &zlog_category_hash_type);
	if (!a_thread->categories) { zc_error("zc_hashtable_new fail"); goto err; }

	zc_hashtable_foreach(a_thread->categories, a_category) {
		rc = zlog_category_flush(a_category);
		if (rc) {zc_error("zlog_category_flush fail"); goto err;}

		b_category = zlog_category_new(a_category, version,
			a_thread->event, a_thread->mdc, b_conf->rules);
		if (!category) { zc_error("zlog_category_new fail"); goto err; }

		rc = zc_hashtable_put(b_categories, b_category->name, b_category);
		if (rc) { zlog_category_del(b_category); zc_error("zc_hashtable_put fail"); goto err; }
	}

	zlog_conf_del(a_thread->conf);
	zc_hashtable_del(a_thread->categories);

	a_thread->version = version;
	a_thread->conf = b_conf;
	a_thread->categories = b_categories;
	return 0;
err:
	if (b_conf) zlog_conf_del(b_conf);
	if (b_categories) zc_hashtable_del(b_categories);
	return -1;
}
/*******************************************************************************/
