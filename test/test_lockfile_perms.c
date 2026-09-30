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

/* The rotation lock file is created 0644, and a symbolic link in its place is
 * refused rather than followed.
 *
 * It used to be created with S_IRWXU|S_IRWXG|S_IRWXO and no O_NOFOLLOW, which
 * matters most for the default path when there is no configuration file to
 * lock against: /tmp/zlog.lock is shared by every such process, and whoever
 * gets there first decides what the name points at.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "zlog.h"

#define CONF     "lockfile_perms.conf"
#define LOGFILE  "lockfile_perms.log"
#define LOCKFILE "lockfile_perms.lock"
#define TARGET   "lockfile_perms_target.txt"
#define ERRLOG   "lockfile_perms_err.txt"

static void cleanup(void)
{
	remove(CONF);
	remove(LOGFILE);
	remove(LOGFILE ".0");
	remove(LOCKFILE);
	remove(TARGET);
	remove(ERRLOG);
}

static int write_conf(void)
{
	FILE *fp = fopen(CONF, "w");

	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return -1;
	}
	fprintf(fp, "[global]\n"
		    "rotate lock file = %s\n"
		    "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"%s\", 1KB * 3; simple\n", LOCKFILE, LOGFILE);
	fclose(fp);
	return 0;
}

static int log_past_rotation(void)
{
	zlog_category_t *c;
	int i;

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		return -1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		return -1;
	}

	for (i = 0; i < 200; i++)
		zlog_info(c, "%04d 0123456789012345678901234567890123456789", i);

	zlog_fini();
	return 0;
}

int main(void)
{
	struct stat stb;
	mode_t perms;

	cleanup();

	/* a known umask, so the expected mode is exactly 0644 */
	umask(0);

	if (write_conf())
		return 1;

	if (log_past_rotation()) {
		cleanup();
		return 1;
	}

	if (stat(LOCKFILE, &stb)) {
		fprintf(stderr, "no %s: was the lock never taken?\n", LOCKFILE);
		cleanup();
		return 1;
	}

	perms = stb.st_mode & 07777;
	if (perms != 0644) {
		fprintf(stderr, "lock file is %04o, expected 0644\n",
			(unsigned)perms);
		cleanup();
		return 1;
	}

	/* now put a symbolic link where the lock file goes */
	remove(LOCKFILE);
	remove(LOGFILE);
	remove(LOGFILE ".0");

	if (fclose(fopen(TARGET, "w"))) {
		fprintf(stderr, "cannot write %s\n", TARGET);
		cleanup();
		return 1;
	}
	if (symlink(TARGET, LOCKFILE)) {
		fprintf(stderr, "cannot create the symlink\n");
		cleanup();
		return 1;
	}

	if (setenv("ZLOG_PROFILE_ERROR", ERRLOG, 1)) {
		fprintf(stderr, "setenv fail\n");
		cleanup();
		return 1;
	}

	if (log_past_rotation()) {
		cleanup();
		return 1;
	}

#ifdef O_NOFOLLOW
	/* the open is refused, so the rotation does not happen */
	if (!access(LOGFILE ".0", F_OK)) {
		fprintf(stderr, "rotation happened through a symlinked lock "
				"file\n");
		cleanup();
		return 1;
	}
#endif

	cleanup();

	printf("lockfile perms tests passed\n");
	return 0;
}
