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

/* zlog_is_init() tells a library whether the program around it has already set
 * zlog up, so it can leave zlog_init() to whoever owns it
 * (HardySimpson/zlog#148).
 */

#include <stdio.h>
#include <stdlib.h>

#include "zlog.h"

#define CONF "is_init.conf"

static int check(const char *when, int want)
{
	int got = zlog_is_init();

	/* non-zero, not any particular value */
	if (!got != !want) {
		fprintf(stderr, "zlog_is_init() is %d %s, expected %s\n",
			got, when, want ? "non-zero" : "zero");
		return -1;
	}
	return 0;
}

int main(void)
{
	FILE *fp;

	remove(CONF);

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fprintf(fp, "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    >stdout; simple\n");
	fclose(fp);

	if (check("before zlog_init()", 0))
		goto err;

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		goto err;
	}

	if (check("after zlog_init()", 1)) {
		zlog_fini();
		goto err;
	}

	/* a reload leaves zlog initialised */
	if (zlog_reload(CONF)) {
		fprintf(stderr, "zlog_reload fail\n");
		zlog_fini();
		goto err;
	}

	if (check("after zlog_reload()", 1)) {
		zlog_fini();
		goto err;
	}

	zlog_fini();

	if (check("after zlog_fini()", 0))
		goto err;

	/* and the same through the default category interface */
	if (dzlog_init(CONF, "my_cat")) {
		fprintf(stderr, "dzlog_init fail\n");
		goto err;
	}

	if (check("after dzlog_init()", 1)) {
		zlog_fini();
		goto err;
	}

	zlog_fini();

	if (check("after the second zlog_fini()", 0))
		goto err;

	remove(CONF);

	printf("is init tests passed\n");
	return 0;

err:
	remove(CONF);
	return 1;
}
