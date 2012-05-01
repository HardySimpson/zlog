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

void zlog_category_profile(zlog_category_t *a_category, int flag)
{
	int i;
	zlog_rule_t *a_rule;

	zc_assert(a_category,);
	zc_profile(flag, "--category[%p][%s][%p]--",
			a_category,
			a_category->name,
			a_category->match_rules);
	if (a_category->match_rules) {
		zc_arraylist_foreach(a_category->match_rules, i, a_rule) {
			zlog_rule_profile(a_rule, flag);
		}
	}
	return;
}

/*******************************************************************************/
void zlog_category_del(zlog_category_t * a_category)
{
	zc_assert(a_category,);
	if (a_category->match_rules) zc_arraylist_del(a_category->match_rules);
	free(a_category);
	zc_debug("zlog_category_del[%p]", a_category);
	return;
}

static int zlog_category_set_name(zlog_category_t * a_category, char *name)
{
	size_t len;

	len = strlen(name);
	if (len > sizeof(a_category->name) - 1) {
		zc_error("name[%d] too long", name);
		return -1;
	}

	memset(a_category->name, 0x00, sizeof(a_category->name));
	strcpy(a_category->name, name);
	a_category->name_len = len;
	return 0;
}

int zlog_category_obtain_rules(zlog_category_t * a_category, zc_arraylist_t * rules)
{
	int i;
	int count = 0;
	int rc = 0;
	int match = 0;
	zlog_rule_t *a_rule;
	zlog_rule_t *wastebin_rule = NULL;

	if (a_category->match_rules) {
		/* before set, clean last match rules first */
		zc_arraylist_del(a_category->match_rules);
	}

	a_category->match_rules = zc_arraylist_new(NULL);
	if (!(a_category->match_rules)) {
		zc_error("zc_arraylist_new fail");
		return -1;
	}

	/* get match rules from all rules */
	zc_arraylist_foreach(rules, i, a_rule) {
		match = zlog_rule_match_category(a_rule, a_category->name);
		if (match) {
			rc = zc_arraylist_add(a_category->match_rules, a_rule);
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
			zc_debug("category[%s], no match rules, use wastebin_rule", a_category->name);
			rc = zc_arraylist_add(a_category->match_rules, wastebin_rule);
			if (rc) {
				zc_error("zc_arrylist_add fail");
				goto zlog_category_match_rules_exit;
			}
			count++;
		} else {
			zc_debug("category[%s], no match rules & no wastebin_rule", a_category->name);
		}
	}

      zlog_category_match_rules_exit:
	if (rc) {
		zc_arraylist_del(a_category->match_rules);
		a_category->match_rules = NULL;
		return -1;
	} else {
		return 0;
	}
}

zlog_category_t *zlog_category_new(char *name, zc_arraylist_t * rules)
{
	int rc = 0;
	zlog_category_t *a_category;

	zc_assert(name, NULL);

	a_category = calloc(1, sizeof(zlog_category_t));
	if (!a_category) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	rc = zlog_category_set_name(a_category, name);
	if (rc) {
		zc_error("zlog_category_set_name fail");
		goto zlog_category_new_exit;
	}

	rc = zlog_category_match_rules(a_category, rules);
	if (rc) {
		zc_error("zlog_category_match_rules fail");
		goto zlog_category_new_exit;
	}

      zlog_category_new_exit:
	if (rc) {
		zlog_category_del(a_category);
		return NULL;
	} else {
		zlog_category_profile(a_category, ZC_DEBUG);
		return a_category;
	}
}

int zlog_category_output(zlog_category_t * a_category, zlog_thread_t * a_thread)
{
	int i;
	int rc = 0;
	int rd = 0;
	zlog_rule_t *a_rule;

	/* go through all match rules to output */
	zc_arraylist_foreach(a_category->match_rules, i, a_rule) {
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
