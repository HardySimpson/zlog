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

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "category.h"
#include "rule.h"
#include "zc_defs.h"

/*******************************************************************************/
static void zlog_category_del(zlog_category_t * a_cat)
{
	if (a_cat->match_rules)
		zc_arraylist_del(a_cat->match_rules);

	zc_debug("free a_cat at [%p]", a_cat);
	free(a_cat);
	return;
}

static int zlog_category_set_name(zlog_category_t * a_cat, char *name)
{
	size_t len;

	len = strlen(name);
	if (len > sizeof(a_cat->name) - 1) {
		zc_error("name[%d] too long", name);
		return -1;
	}

	memset(a_cat->name, 0x00, sizeof(a_cat->name));
	strcpy(a_cat->name, name);
	a_cat->name_len = len;
	return 0;
}

static int zlog_category_match_rules(zlog_category_t * a_cat,
				     zc_arraylist_t * rules)
{
	int i;
	int count = 0;
	int rc = 0;
	int match = 0;
	zlog_rule_t *a_rule;
	zlog_rule_t *wastebin_rule = NULL;

	if (a_cat->match_rules) {
		/* before set, clean last match rules first */
		zc_arraylist_del(a_cat->match_rules);
	}

	a_cat->match_rules = zc_arraylist_new(NULL);
	if (!(a_cat->match_rules)) {
		zc_error("init match_rule_list fail");
		return -1;
	}

	/* get match rules from all rules */
	zc_arraylist_foreach(rules, i, a_rule) {
		match = zlog_rule_match_category(a_rule, a_cat->name);
		if (match) {
			rc = zc_arraylist_add(a_cat->match_rules, a_rule);
			if (rc) {
				zc_error("zc_arrylist_add fail");
				goto zlog_category_match_rules_exit;
			}
			count++;
		}

		if (zlog_rule_is_wastebin(a_rule)) {
			wastebin_rule = a_rule;
		}
	}

	if (count == 0) {
		if (wastebin_rule) {
			zc_debug("category[%s], no match rules, use wastebin_rule", a_cat->name);
			rc = zc_arraylist_add(a_cat->match_rules, wastebin_rule);
			if (rc) {
				zc_error("zc_arrylist_add fail");
				goto zlog_category_match_rules_exit;
			}
			count++;
		} else {
			zc_debug("category[%s], no match rules & no wastebin_rule", a_cat->name);
		}
	}

      zlog_category_match_rules_exit:
	if (rc) {
		zc_arraylist_del(a_cat->match_rules);
		a_cat->match_rules = NULL;
		return -1;
	} else {
		return 0;
	}
}

static zlog_category_t *zlog_category_new(char *name, zc_arraylist_t * rules)
{
	int rc = 0;
	zlog_category_t *a_cat;

	a_cat = calloc(1, sizeof(zlog_category_t));
	if (!a_cat) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	rc = zlog_category_set_name(a_cat, name);
	if (rc) {
		zc_error("zlog_category_set_name fail");
		goto zlog_category_new_exit;
	}

	rc = zlog_category_match_rules(a_cat, rules);
	if (rc) {
		zc_error("zlog_category_match_rules fail");
		goto zlog_category_new_exit;
	}

      zlog_category_new_exit:
	if (rc) {
		zlog_category_del(a_cat);
		return NULL;
	} else {
		zc_debug("new a_cat at %p", a_cat);
		return a_cat;
	}
}

int zlog_category_output(zlog_category_t * a_cat, zlog_thread_t * a_thread)
{
	int i;
	int rc = 0;
	int rd = 0;
	zlog_rule_t *a_rule;

	/* go through all match rules to output */
	zc_arraylist_foreach(a_cat->match_rules, i, a_rule) {
		rc = zlog_rule_output(a_rule, a_thread);
		if (rc) {
			zc_error("hzb_log_rule_output fail");
			rd = -1;
			/* one rule fail, maybe other will success */
			continue;
		}
	}

	return rd;
}

/*******************************************************************************/
int zlog_cmap_init(zlog_cmap_t * a_cmap)
{
	zc_hashtable_t *a_tab;

	a_tab = zc_hashtable_new(20,
				 (zc_hashtable_hash_fn) zc_hashtable_str_hash,
				 (zc_hashtable_equal_fn) zc_hashtable_str_equal,
				 NULL, (zc_hashtable_del_fn) zlog_category_del);
	if (!a_tab) {
		zc_error("init hashtable fail");
		return -1;
	} else {
		a_cmap->tab = a_tab;
		return 0;
	}
}

void zlog_cmap_fini(zlog_cmap_t * a_cmap)
{
	if (a_cmap->tab)
		zc_hashtable_del(a_cmap->tab);

	memset(a_cmap, 0x00, sizeof(zlog_cmap_t));
	return;
}

int zlog_cmap_update(zlog_cmap_t * a_cmap, zc_arraylist_t * rules)
{
	int rc = 0;
	zc_hashtable_entry_t *a_entry;
	zlog_category_t *a_category;

	zc_hashtable_foreach(a_cmap->tab, a_entry) {
		a_category = (zlog_category_t *) a_entry->value;
		rc = zlog_category_match_rules(a_category, rules);
		if (rc) {
			zc_error("zlog_category_match_rules fail");
			return -1;
		}
	}

	return 0;
}

/*******************************************************************************/
zlog_category_t *zlog_cmap_fetch_category(zlog_cmap_t * a_cmap,
					  char *category_name,
					  zc_arraylist_t * rules)
{
	int rc = 0;
	zlog_category_t *a_cat;

	/* 1st find category in global category map */
	a_cat = zc_hashtable_get(a_cmap->tab, category_name);
	if (a_cat) {
		return a_cat;
	}
	/* else not fount, create one */
	a_cat = zlog_category_new(category_name, rules);
	if (!a_cat) {
		zc_error("zc_category_new fail");
		return NULL;
	}

	rc = zc_hashtable_put(a_cmap->tab, a_cat->name, a_cat);
	if (rc) {
		zc_error("zc_hashtable_put fail");
		goto zlog_cmap_fetch_category_exit;
	}

      zlog_cmap_fetch_category_exit:
	if (rc) {
		zlog_category_del(a_cat);
		return NULL;
	} else {
		return a_cat;
	}
}

/*******************************************************************************/
void zlog_cmap_profile(zlog_cmap_t * a_cmap)
{
	zc_hashtable_entry_t *a_entry;
	zlog_category_t *a_category;

	zc_error("---cmap[%p]---", a_cmap);
	zc_hashtable_foreach(a_cmap->tab, a_entry) {
		a_category = (zlog_category_t *) a_entry->value;
		zc_error("category:[%s]", a_category->name);
	}
	return;
}
