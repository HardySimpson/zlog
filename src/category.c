/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */
#include "fmacros.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "zc_defs.h"

          #include "event.h"
          #include "mdc.h"
        #include "thread.h"
      #include "format.h"
      #include "rotater.h"
    #include "conf.h"
  #include "rule.h"
#include "category.h"

void zlog_category_profile(zlog_category_t *a_category, int flag)
{
	int i;
	zlog_rule_t *a_rule;

	zc_assert(a_category,);
	zc_profile(flag, "--category[%p][%s][%p]--",
			a_category,
			a_category->name,
			a_category->fit_rules);
	if (a_category->fit_rules) {
		zc_arraylist_foreach(a_category->fit_rules, i, a_rule) {
			zlog_rule_profile(a_rule, flag);
		}
	}
	return;
}

/*******************************************************************************/
void zlog_category_del(zlog_category_t * a_category)
{
	zc_assert(a_category,);
	if (a_category->name) zc_sdsfree(a_category->name);
	if (a_category->rules) zc_arraylist_del(a_category->rules);
	free(a_category);
	zc_debug("zlog_category_del[%p]", a_category);
	return;
}

int zlog_cateogry_add_rule(zlog_category_t * a_category, zlog_rule_t *a_rule)
{
	int i;
	zlog_rule_t *b_rule;

	zc_assert(a_category,-1);
	zc_assert(a_rule,-1);

	if (zlog_rule_match_category_name(a_rule, a_category->name)) {
		b_rule = zlog_rule_dup(a_rule);
		if (!b_rule) { zc_error("zlog_rule_dup fail"); return -1; }

		rc = zc_arraylist_add(a_category->rules, b_rule, NULL);
		if (rc) { zc_error("zc_arraylist_add fail"); return -1; }

		for(i = 0; i < sizeof(b_rule->level_bitmap); i++) {
			a_category->level_bitmap[i] |= b_rule->level_bitmap[i];
		}
	}
}

zlog_category_t *zlog_category_new(const char *name, zlog_event_t *event, zlog_mdc_t *mdc);
{
	int rc;
	int count = 0;
	zlog_category_t *a_category;

	a_category = calloc(1, sizeof(zlog_category_t));
	if (!a_category) { zc_error("calloc fail, errno[%d]", errno); return NULL; }

	a_category->name = zc_sdsnew(name);
	if (!a_category) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }
	a_category->mdc = mdc;
	a_category->event = event;

	a_category->rules = zc_arraylist_new();
	if (!(a_category->rules)) { zc_error("zc_arraylist_new fail"); goto err; }
	zc_arraylist_set_del(a_category->rules, zlog_rule_del);

	zlog_category_profile(a_category, ZC_DEBUG);
	return a_category;
err:
	zlog_category_del(a_category);
	return NULL;
}
/*******************************************************************************/

int zlog_category_output(zlog_category_t * a_category)
{
	int i;
	int rc = 0;
	zlog_rule_t *a_rule;

	/* go through all match rules to output */
	zc_arraylist_foreach(a_category->fit_rules, i, a_rule) {
		if (zlog_rule_has_level(a_rule, a_category->event->level)) {
			rc |= zlog_rule_output(a_rule, a_category->event, a_category->mdc);
		}
	}

	return rc;
}
