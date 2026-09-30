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

/* A rotating rule keeps its file open between messages, so it has to notice
 * when the file at the path is no longer the one it holds: after its own
 * rotation, and after an external tool has moved it (HardySimpson/zlog#136).
 *
 * Opening and closing around every write made that impossible to get wrong and
 * cost two syscalls a line.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zlog.h"

#define CONF     "rotate_reopen.conf"
#define LOGFILE  "rotate_reopen.log"
#define MOVED    "rotate_reopen.log.moved"
#define ARCHIVE  LOGFILE ".0"

static void cleanup(void)
{
	remove(CONF);
	remove(LOGFILE);
	remove(MOVED);
	remove(ARCHIVE);
	remove(LOGFILE ".1");
	remove(LOGFILE ".2");
}

static int file_has(const char *path, const char *needle)
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

	cleanup();

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fprintf(fp, "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"%s\", 4KB * 3; simple\n", LOGFILE);
	fclose(fp);

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
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

	/* well past 4KB: the rule rotates, and has to pick up the new file it
	 * created rather than keep writing to the archive */
	for (i = 0; i < 300; i++)
		zlog_info(c, "%04d 0123456789012345678901234567890123456789", i);

	if (access(ARCHIVE, F_OK)) {
		fprintf(stderr, "no %s: rotation did not happen\n", ARCHIVE);
		zlog_fini();
		cleanup();
		return 1;
	}

	if (!file_has(LOGFILE, "0299 ")) {
		fprintf(stderr, "the last message is not in %s: the rule kept "
				"writing to the file it had rotated away\n", LOGFILE);
		zlog_fini();
		cleanup();
		return 1;
	}

	/* now move it aside the way logrotate would, with zlog still running */
	if (rename(LOGFILE, MOVED)) {
		fprintf(stderr, "cannot rename %s\n", LOGFILE);
		zlog_fini();
		cleanup();
		return 1;
	}

	for (i = 0; i < 10; i++)
		zlog_info(c, "after-the-move-%02d", i);

	zlog_fini();

	if (access(LOGFILE, F_OK)) {
		fprintf(stderr, "no %s after the rename: the rule kept writing "
				"to the file that was moved away\n", LOGFILE);
		cleanup();
		return 1;
	}

	if (!file_has(LOGFILE, "after-the-move-09")) {
		fprintf(stderr, "%s does not hold the messages logged after the "
				"rename\n", LOGFILE);
		cleanup();
		return 1;
	}

	if (file_has(MOVED, "after-the-move-09")) {
		fprintf(stderr, "the messages logged after the rename went to "
				"the moved file\n");
		cleanup();
		return 1;
	}

	cleanup();

	printf("rotate reopen tests passed\n");
	return 0;
}
