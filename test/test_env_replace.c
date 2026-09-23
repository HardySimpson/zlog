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

/* Every %E(NAME) in a format or a file path has to be replaced, not just the
 * first one.
 *
 * zc_str_replace_env() moved the tail of the string over the substitution but
 * went on searching from where the tail used to start, so with a value shorter
 * than %E(NAME) the next pass began somewhere inside the tail. A second
 * %E(NAME) was then left in the string, and a format or a path that still
 * holds one fails zlog_init() (HardySimpson/zlog#89).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zlog.h"

#define CONF    "env_replace.conf"
#define LOGFILE "env_replace_xy.log"

static int file_contains(const char *path, const char *needle)
{
	char buf[4096];
	size_t len;
	FILE *fp = fopen(path, "r");

	if (!fp)
		return 0;
	memset(buf, 0x00, sizeof(buf));
	len = fread(buf, 1, sizeof(buf) - 1, fp);
	buf[len] = '\0';
	fclose(fp);

	return strstr(buf, needle) != NULL;
}

int main(void)
{
	zlog_category_t *c;
	FILE *fp;

	remove(CONF);
	remove(LOGFILE);

	/* one character each, so the value is shorter than the %E(NAME) it
	 * replaces: that is what used to leave the second one behind */
	if (setenv("ZLOG_TEST_A", "x", 1) || setenv("ZLOG_TEST_B", "y", 1)) {
		fprintf(stderr, "setenv fail\n");
		return 1;
	}

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	/* twice in the format, twice in the path: format.c and rule.c each
	 * call zc_str_replace_env() on their own string */
	fprintf(fp, "[formats]\n"
		    "simple = \"%%E(ZLOG_TEST_A)%%E(ZLOG_TEST_B)|%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"env_replace_%%E(ZLOG_TEST_A)%%E(ZLOG_TEST_B).log\";"
		    " simple\n");
	fclose(fp);

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail: a %%E() left in the format or "
				"the path is a configuration error\n");
		remove(CONF);
		return 1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		remove(CONF);
		return 1;
	}

	zlog_info(c, "hello");
	zlog_fini();

	/* the path: both variables expanded, or this name does not exist */
	if (access(LOGFILE, F_OK)) {
		fprintf(stderr, "no %s: the second %%E() in the path was not "
				"replaced\n", LOGFILE);
		remove(CONF);
		return 1;
	}

	/* the format: "xy|hello", not "x%E(ZLOG_TEST_B)|hello" */
	if (!file_contains(LOGFILE, "xy|hello")) {
		fprintf(stderr, "the second %%E() in the format was not "
				"replaced\n");
		remove(CONF);
		remove(LOGFILE);
		return 1;
	}

	remove(CONF);
	remove(LOGFILE);

	printf("env replace tests passed\n");
	return 0;
}
