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

/* zlog_fini_try() must return instead of waiting for the configuration lock.
 *
 * zlog_fini() from a signal handler that interrupted a thread inside zlog
 * hangs: the handler runs on that thread, which already holds the lock, and a
 * pthread rwlock is not recursive (HardySimpson/zlog#49). The record callback
 * below stands in for the handler -- it runs from inside zlog_info(), with the
 * read lock held by this very thread, which is the shape that deadlocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

#define CONF     "fini_try.conf"

static int fini_rc = -2;

static int record_output(zlog_msg_t *msg)
{
	(void) msg;

	/* the lock is held right now, one frame up */
	fini_rc = zlog_fini_try();
	return 0;
}

int main(void)
{
	zlog_category_t *c;
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
		    "my_cat.*    $myoutput, \"path\"; simple\n");
	fclose(fp);

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		remove(CONF);
		return 1;
	}

	if (zlog_set_record("myoutput", record_output)) {
		fprintf(stderr, "zlog_set_record fail\n");
		zlog_fini();
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

	/* if zlog_fini_try() blocked, this never comes back and the test times
	 * out rather than failing -- which is the bug it is about */
	zlog_info(c, "hello");

	if (fini_rc != -1) {
		fprintf(stderr, "zlog_fini_try() returned %d with the lock "
				"held, expected -1\n", fini_rc);
		zlog_fini();
		remove(CONF);
		return 1;
	}

	/* nothing was torn down, so zlog is still usable */
	zlog_info(c, "still here");

	/* and with no lock held it does finish */
	if (zlog_fini_try()) {
		fprintf(stderr, "zlog_fini_try() failed with no lock held\n");
		remove(CONF);
		return 1;
	}

	/* which means a fresh init works */
	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init after zlog_fini_try fail: was the "
				"teardown incomplete?\n");
		remove(CONF);
		return 1;
	}
	zlog_fini();

	/* on an uninitialised zlog it is a no-op, not an error */
	if (zlog_fini_try()) {
		fprintf(stderr, "zlog_fini_try() on an uninitialised zlog "
				"failed\n");
		remove(CONF);
		return 1;
	}

	remove(CONF);

	printf("fini try tests passed\n");
	return 0;
}
