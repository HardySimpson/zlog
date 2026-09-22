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

/* Rotation must still happen when the configuration file is read only.
 *
 * The rotate lock file defaults to the configuration file, which lock_file()
 * opens read-write: a configuration the process may only read -- the usual
 * /etc/zlog.conf -- used to fail the lock on every rotation, so the log grew
 * without bound and said so only under ZLOG_PROFILE_ERROR
 * (HardySimpson/zlog#88, #69).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "zlog.h"

#define CONF     "rotate_readonly.conf"
#define LOGFILE  "rotate_readonly.log"
#define ARCHIVE  LOGFILE ".0"

static void cleanup(void)
{
	chmod(CONF, 0644);
	remove(CONF);
	remove(LOGFILE);
	remove(ARCHIVE);
}

int main(void)
{
	zlog_category_t *c;
	FILE *fp;
	int i;

	/* root writes a 0444 file without asking, so there is nothing to test */
	if (geteuid() == 0) {
		printf("rotate readonly conf test skipped (running as root)\n");
		return 0;
	}

	cleanup();

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fprintf(fp, "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"%s\", 1KB * 3; simple\n", LOGFILE);
	fclose(fp);

	if (chmod(CONF, 0444)) {
		fprintf(stderr, "cannot make %s read only\n", CONF);
		cleanup();
		return 1;
	}

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

	/* well past 1KB, so rotation is due many times over */
	for (i = 0; i < 500; i++)
		zlog_info(c, "%04d 0123456789012345678901234567890123456789", i);

	zlog_fini();

	if (access(ARCHIVE, F_OK)) {
		fprintf(stderr, "no %s: rotation never happened with a read only"
				" configuration file\n", ARCHIVE);
		cleanup();
		return 1;
	}

	cleanup();

	printf("rotate readonly conf tests passed\n");
	return 0;
}
