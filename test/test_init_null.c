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

/* A NULL configuration means "no configuration file": zlog falls back to its
 * built-in default, *.* >stdout, rather than dereferencing the pointer
 * (HardySimpson/zlog#185). ZLOG_CONF_PATH still gets a say, since that is the
 * other way to name a configuration when the caller has none.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

#define CONF    "init_null.conf"
#define LOGFILE "init_null.log"

static void cleanup(void)
{
	remove(CONF);
	remove(LOGFILE);
}

static int file_has(const char *path, const char *needle)
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

	cleanup();

	/* otherwise a configuration named in the environment decides the test */
	unsetenv("ZLOG_CONF_PATH");

	/* 1. the reported call: it must return, not crash */
	if (zlog_init(NULL)) {
		fprintf(stderr, "zlog_init(NULL) failed\n");
		return 1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category after zlog_init(NULL) failed\n");
		zlog_fini();
		return 1;
	}

	/* the built-in default is *.* >stdout, so the category is covered */
	if (!zlog_category_has_rules(c)) {
		fprintf(stderr, "no rules after zlog_init(NULL): the default "
				"configuration was not applied\n");
		zlog_fini();
		return 1;
	}

	zlog_info(c, "init null: the default configuration logs to stdout");

	/* 2. a reload with NULL keeps it that way */
	if (zlog_reload(NULL)) {
		fprintf(stderr, "zlog_reload(NULL) failed\n");
		zlog_fini();
		return 1;
	}
	zlog_info(c, "init null: still here after zlog_reload(NULL)");
	zlog_fini();

	/* 3. the same through the default category interface */
	if (dzlog_init(NULL, "my_cat")) {
		fprintf(stderr, "dzlog_init(NULL, ...) failed\n");
		return 1;
	}
	dzlog_info("init null: default category on the default configuration");
	zlog_fini();

	/* 4. with no argument, ZLOG_CONF_PATH names the configuration */
	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fprintf(fp, "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"%s\"; simple\n", LOGFILE);
	fclose(fp);

	if (setenv("ZLOG_CONF_PATH", CONF, 1)) {
		fprintf(stderr, "setenv fail\n");
		cleanup();
		return 1;
	}

	if (zlog_init(NULL)) {
		fprintf(stderr, "zlog_init(NULL) with ZLOG_CONF_PATH set failed\n");
		cleanup();
		return 1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		cleanup();
		return 1;
	}

	zlog_info(c, "from the configuration named in the environment");
	zlog_fini();
	unsetenv("ZLOG_CONF_PATH");

	if (!file_has(LOGFILE, "from the configuration named in the environment")) {
		fprintf(stderr, "ZLOG_CONF_PATH was ignored: nothing in %s\n",
			LOGFILE);
		cleanup();
		return 1;
	}

	cleanup();

	printf("init null tests passed\n");
	return 0;
}
