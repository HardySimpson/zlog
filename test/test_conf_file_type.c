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

/* What zlog_init() accepts as a conf file.
 *
 * A directory used to be accepted: nothing could be read from it, so the
 * logger came up configured as if the file were empty and silently threw
 * every message away (#278). It is rejected now, and the point of this test
 * is as much what stays accepted -- a symlink to a regular file, which is why
 * the check has to stat() rather than lstat().
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "zlog.h"

#define CONF "conf_file_type.conf"
#define DIR  "conf_file_type_dir"
#define LINK_TO_CONF "conf_file_type_link.conf"
#define LINK_TO_DIR  "conf_file_type_link_dir.conf"
#define OUT  "conf_file_type.txt"

static int fails(const char *path)
{
	if (zlog_init(path) == 0) {
		fprintf(stderr, "zlog_init(%s) should have failed\n", path);
		zlog_fini();
		return -1;
	}
	return 0;
}

/* accepted, and the rules in the file are the ones in force */
static int works(const char *path)
{
	zlog_category_t *c;
	char got[64];
	size_t len;
	FILE *fp;

	remove(OUT);

	if (zlog_init(path)) {
		fprintf(stderr, "zlog_init(%s) should have worked\n", path);
		return -1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category fail for %s\n", path);
		zlog_fini();
		return -1;
	}

	zlog_info(c, "hello");
	zlog_fini();

	memset(got, 0x00, sizeof(got));
	fp = fopen(OUT, "r");
	if (!fp) {
		fprintf(stderr, "%s logged nothing through %s\n", OUT, path);
		return -1;
	}
	len = fread(got, 1, sizeof(got) - 1, fp);
	got[len] = '\0';
	fclose(fp);

	if (strcmp(got, "hello\n")) {
		fprintf(stderr, "through %s, %s holds [%s]\n", path, OUT, got);
		return -1;
	}
	return 0;
}

int main(void)
{
	FILE *fp;

	remove(LINK_TO_CONF);
	remove(LINK_TO_DIR);
	remove(CONF);
	rmdir(DIR);

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fputs("[formats]\nsimple = \"%m%n\"\n[rules]\nmy_cat.*    \"" OUT "\"; simple\n", fp);
	fclose(fp);

	if (mkdir(DIR, 0755)) {
		fprintf(stderr, "cannot create %s\n", DIR);
		return 1;
	}

	/* a regular file, the ordinary case */
	if (works(CONF))
		return 1;

	/* a directory, which used to come up as an empty config */
	if (fails(DIR))
		return 1;

	/* and one that is not there at all */
	if (fails("conf_file_type_missing.conf"))
		return 1;

#ifndef _WIN32
	/* a character device is no more readable as a config than a directory */
	if (fails("/dev/null"))
		return 1;

	if (symlink(CONF, LINK_TO_CONF) || symlink(DIR, LINK_TO_DIR)) {
		fprintf(stderr, "cannot create symlinks\n");
		return 1;
	}

	/* a symlink to a regular file must keep working: the check follows
	 * symlinks, so it sees the target rather than the link itself */
	if (works(LINK_TO_CONF))
		return 1;

	/* a symlink to a directory is still a directory */
	if (fails(LINK_TO_DIR))
		return 1;
#endif

	printf("conf file type tests passed\n");
	return 0;
}
