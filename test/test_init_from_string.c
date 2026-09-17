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

/* zlog_init_from_string() and zlog_reload_from_string().
 *
 * Nothing else in this directory uses the string API, which is how the leak
 * in HardySimpson/zlog#256 survived: the conf builds a rotater and a default
 * format from the defaults before parsing, and [rules] builds both again from
 * the settings [global] has changed by then. Under the Asan build the leak
 * fails this test on its own; the checks below pin the other half, that the
 * pair in effect is the one built from the string rather than the default.
 */

#include <stdio.h>
#include <string.h>

#include "zlog.h"

static const char conf_a[] =
	"[global]\n"
	"default format = \"A %m%n\"\n"
	"\n"
	"[rules]\n"
	"my_cat.*    \"init_from_string_a.txt\"\n";

static const char conf_b[] =
	"[global]\n"
	"default format = \"B %m%n\"\n"
	"\n"
	"[rules]\n"
	"my_cat.*    \"init_from_string_b.txt\"\n";

static int file_is(const char *path, const char *want)
{
	char got[128];
	size_t len;
	FILE *fp = fopen(path, "r");

	memset(got, 0x00, sizeof(got));
	if (!fp) {
		fprintf(stderr, "%s not written\n", path);
		return -1;
	}
	len = fread(got, 1, sizeof(got) - 1, fp);
	got[len] = '\0';
	fclose(fp);

	if (strcmp(got, want)) {
		fprintf(stderr, "%s holds [%s], want [%s]\n", path, got, want);
		return -1;
	}
	return 0;
}

int main(void)
{
	zlog_category_t *c;
	int i;

	remove("init_from_string_a.txt");
	remove("init_from_string_b.txt");

	if (zlog_init_from_string(conf_a)) {
		fprintf(stderr, "zlog_init_from_string fail\n");
		return 1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		return 1;
	}

	zlog_info(c, "one");

	/* the rule has no format of its own, so this is the default format the
	 * string asked for, not the built in one */
	if (file_is("init_from_string_a.txt", "A one\n")) {
		zlog_fini();
		return 1;
	}

	/* reloading goes through the same build, and leaked the same pair */
	for (i = 0; i < 5; i++) {
		if (zlog_reload_from_string(conf_b)) {
			fprintf(stderr, "zlog_reload_from_string fail\n");
			zlog_fini();
			return 1;
		}
	}

	c = zlog_get_category("my_cat");
	zlog_info(c, "two");

	if (file_is("init_from_string_b.txt", "B two\n")) {
		zlog_fini();
		return 1;
	}

	zlog_fini();
	printf("init from string tests passed\n");
	return 0;
}
