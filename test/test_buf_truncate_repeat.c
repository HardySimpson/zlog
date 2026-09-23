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

/* A message that overflows the buffer must come out the same every time.
 *
 * zlog_buf_vprintf() only formats a second time when the resize that failed
 * still won some room; the first message grows the buffer to buffer_max and
 * takes that path, every message after it finds the buffer already at the
 * maximum and keeps what the first vsnprintf() wrote. The two lines here have
 * to be identical, which they are not if that shortcut keeps the wrong length
 * (HardySimpson/zlog#95).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

#define CONF     "buf_truncate_repeat.conf"
#define LOGFILE  "buf_truncate_repeat.log"

#define MSG_LEN  (64 * 1024)
#define WIDTH    200

int main(void)
{
	zlog_category_t *c;
	FILE *fp;
	char *msg;
	char line1[WIDTH + 2];
	char line2[WIDTH + 2];
	int i;

	remove(CONF);
	remove(LOGFILE);

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	/* a maximum well under the message, so the buffer grows once and is
	 * then stuck at buffer max for every message after the first */
	fprintf(fp, "[global]\n"
		    "buffer min = 1K\n"
		    "buffer max = 4K\n"
		    "[formats]\n"
		    "cut = \"%%.200m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"%s\"; cut\n", LOGFILE);
	fclose(fp);

	msg = malloc(MSG_LEN + 1);
	if (!msg) {
		fprintf(stderr, "malloc fail\n");
		remove(CONF);
		return 1;
	}
	memset(msg, 'x', MSG_LEN);
	msg[MSG_LEN] = '\0';

	if (zlog_init(CONF)) {
		fprintf(stderr, "zlog_init fail\n");
		free(msg);
		remove(CONF);
		return 1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		free(msg);
		remove(CONF);
		return 1;
	}

	zlog_info(c, "%s", msg);	/* grows the buffer to buffer max */
	zlog_info(c, "%s", msg);	/* finds it there already */
	zlog_fini();
	free(msg);

	fp = fopen(LOGFILE, "r");
	if (!fp) {
		fprintf(stderr, "no %s at all\n", LOGFILE);
		remove(CONF);
		return 1;
	}
	memset(line1, 0x00, sizeof(line1));
	memset(line2, 0x00, sizeof(line2));
	if (!fgets(line1, sizeof(line1), fp) || !fgets(line2, sizeof(line2), fp)) {
		fprintf(stderr, "expected two lines in %s\n", LOGFILE);
		fclose(fp);
		remove(CONF);
		remove(LOGFILE);
		return 1;
	}
	fclose(fp);

	/* fgets keeps the newline; the width is about the message */
	if (strlen(line1) && line1[strlen(line1) - 1] == '\n')
		line1[strlen(line1) - 1] = '\0';
	if (strlen(line2) && line2[strlen(line2) - 1] == '\n')
		line2[strlen(line2) - 1] = '\0';

	if (strcmp(line1, line2)) {
		fprintf(stderr, "the two lines differ: [%lu] then [%lu] bytes\n",
			(unsigned long)strlen(line1), (unsigned long)strlen(line2));
		remove(CONF);
		remove(LOGFILE);
		return 1;
	}

	if (strlen(line1) != WIDTH) {
		fprintf(stderr, "line is %lu bytes, %%.200m asks for %d\n",
			(unsigned long)strlen(line1), WIDTH);
		remove(CONF);
		remove(LOGFILE);
		return 1;
	}

	/* the truncation mark belongs past the width, never inside it */
	for (i = 0; i < WIDTH; i++) {
		if (line1[i] != 'x') {
			fprintf(stderr, "byte %d of the line is [%c], not part of "
					"the message\n", i, line1[i]);
			remove(CONF);
			remove(LOGFILE);
			return 1;
		}
	}

	remove(CONF);
	remove(LOGFILE);

	printf("buf truncate repeat tests passed\n");
	return 0;
}
