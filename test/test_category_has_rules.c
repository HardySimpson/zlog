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

/* zlog_get_category() creates a category for any name, so it cannot say
 * whether the configuration mentions it. zlog_category_has_rules() can
 * (HardySimpson/zlog#95).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

#define CONF       "category_has_rules.conf"
#define WILD_CONF  "category_has_rules_wild.conf"
#define LOGFILE    "category_has_rules.log"

static int write_conf(const char *path, const char *rule_cat)
{
	FILE *fp = fopen(path, "w");

	if (!fp) {
		fprintf(stderr, "cannot write %s\n", path);
		return -1;
	}
	fprintf(fp, "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "%s.*    \"%s\"; simple\n", rule_cat, LOGFILE);
	fclose(fp);
	return 0;
}

static void cleanup(void)
{
	remove(CONF);
	remove(WILD_CONF);
	remove(LOGFILE);
}

int main(void)
{
	zlog_category_t *known;
	zlog_category_t *unknown;

	cleanup();

	/* a rule for my_cat and nothing else */
	if (write_conf(CONF, "my_cat"))
		return 1;

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		cleanup();
		return 1;
	}

	known = zlog_get_category("my_cat");
	unknown = zlog_get_category("no_such_cat");

	if (!known || !unknown) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		cleanup();
		return 1;
	}

	if (!zlog_category_has_rules(known)) {
		fprintf(stderr, "my_cat is in the configuration and was not "
				"reported as such\n");
		zlog_fini();
		cleanup();
		return 1;
	}

	if (zlog_category_has_rules(unknown)) {
		fprintf(stderr, "no_such_cat is in no rule and was reported as "
				"configured\n");
		zlog_fini();
		cleanup();
		return 1;
	}

	if (zlog_category_has_rules(NULL)) {
		fprintf(stderr, "NULL was reported as configured\n");
		zlog_fini();
		cleanup();
		return 1;
	}

	zlog_fini();

	/* a wildcard rule covers every name, including one nothing names */
	if (write_conf(WILD_CONF, "*"))
		return 1;

	if (zlog_init(WILD_CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		cleanup();
		return 1;
	}

	unknown = zlog_get_category("no_such_cat");
	if (!unknown) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		cleanup();
		return 1;
	}

	if (!zlog_category_has_rules(unknown)) {
		fprintf(stderr, "a wildcard rule covers every category, so this "
				"one has rules too\n");
		zlog_fini();
		cleanup();
		return 1;
	}

	zlog_fini();
	cleanup();

	printf("category has rules tests passed\n");
	return 0;
}
