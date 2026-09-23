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

/* A message longer than buffer max is truncated, not dropped.
 *
 * zlog_buf_vprintf() truncates and says so with a return of 1, but
 * zlog_format_gen_msg() used to treat anything non-zero as a failure, and the
 * rules turn a failed format into no output at all: the whole line went
 * missing, and only ZLOG_PROFILE_ERROR said why (HardySimpson/zlog#95).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

#define CONF     "buf_max.conf"
#define LOGFILE  "buf_max.log"

#define BUF_MAX  2048
#define MSG_LEN  8000

int main(void)
{
	zlog_category_t *c;
	FILE *fp;
	char *msg;
	char out[BUF_MAX * 2];
	size_t len;

	remove(CONF);
	remove(LOGFILE);

	fp = fopen(CONF, "w");
	if (!fp) {
		fprintf(stderr, "cannot write %s\n", CONF);
		return 1;
	}
	fprintf(fp, "[global]\n"
		    "buffer min = 1K\n"
		    "buffer max = 2K\n"
		    "[formats]\n"
		    "simple = \"%%m%%n\"\n"
		    "[rules]\n"
		    "my_cat.*    \"%s\"; simple\n", LOGFILE);
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

	zlog_info(c, "%s", msg);
	zlog_fini();
	free(msg);

	fp = fopen(LOGFILE, "r");
	if (!fp) {
		fprintf(stderr, "no %s at all\n", LOGFILE);
		remove(CONF);
		return 1;
	}
	memset(out, 0x00, sizeof(out));
	len = fread(out, 1, sizeof(out) - 1, fp);
	fclose(fp);

	if (len == 0) {
		fprintf(stderr, "%s is empty: a message over buffer max was "
				"dropped instead of truncated\n", LOGFILE);
		remove(CONF);
		remove(LOGFILE);
		return 1;
	}

	if (len > BUF_MAX) {
		fprintf(stderr, "wrote %lu bytes, buffer max is %d\n",
			(unsigned long)len, BUF_MAX);
		remove(CONF);
		remove(LOGFILE);
		return 1;
	}

	/* zlog_buf_new() is given "..." and a newline as the truncation mark */
	if (len < 4 || strcmp(out + len - 4, "...\n")) {
		fprintf(stderr, "the line does not end in the truncation mark: "
				"[%s]\n", out + (len > 16 ? len - 16 : 0));
		remove(CONF);
		remove(LOGFILE);
		return 1;
	}

	remove(CONF);
	remove(LOGFILE);

	printf("buf max truncate tests passed\n");
	return 0;
}
