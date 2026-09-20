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

/* A rotation whose lock file cannot be opened must not wedge the rotater.
 *
 * zlog_rotater_trylock() takes lock_mutex and then opens the lock file. When
 * that open failed it used to return with the mutex still held, so every
 * rotation afterwards failed at the trylock with EBUSY -- rotation was dead
 * for the life of the process, and the log grew without bound
 * (HardySimpson/zlog#300, and the symptom reported in #252).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

#define CONF     "rotate_lock.conf"
#define ERRLOG   "rotate_lock_err.txt"
#define LOGFILE  "rotate_lock.log"

/* a lock file under a directory that does not exist: open() fails every time */
#define BAD_LOCK "rotate_lock_no_such_dir/zlog.lock"

static int contains(const char *path, const char *needle)
{
	char buf[65536];
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
	int i;

	remove(ERRLOG);
	remove(LOGFILE);

	/* zc_error() writes here, and reads the variable once, on first use */
	if (setenv("ZLOG_PROFILE_ERROR", ERRLOG, 1)) {
		fprintf(stderr, "setenv fail\n");
		return 1;
	}

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fprintf(fp, "[global]\n"
		    "rotate lock file = %s\n"
		    "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"%s\", 1KB * 3; simple\n", BAD_LOCK, LOGFILE);
	fclose(fp);

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		return 1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		zlog_fini();
		return 1;
	}

	/* well past 1KB, so rotation is attempted many times over */
	for (i = 0; i < 500; i++)
		zlog_info(c, "%04d 0123456789012345678901234567890123456789", i);

	zlog_fini();

	if (!contains(ERRLOG, "lock file error")) {
		fprintf(stderr, "the lock file was expected to fail to open; "
				"did the rotation never happen?\n");
		return 1;
	}

	/* only reachable if a previous attempt walked away still holding it */
	if (contains(ERRLOG, "locked by other threads")) {
		fprintf(stderr, "lock_mutex was left held after the lock file "
				"failed to open\n");
		return 1;
	}

	remove(CONF);
	remove(ERRLOG);
	remove(LOGFILE);

	printf("rotate lock tests passed\n");
	return 0;
}
