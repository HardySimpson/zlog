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

/* Rules as seen by the consumer thread.
 *
 * Every other consumer conf in this directory uses a "category.*" rule, which
 * needs no level from the message and never reaches an output that wants a
 * calling thread. These cases do:
 *
 *   - a level filtered rule must select the same messages with the writer
 *     thread as without it
 *   - a syslog rule must not be handed a NULL thread
 *   - so must a rotation whose archive path has specs in it
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlog.h"

static int write_conf(const char *path, const char *body)
{
	FILE *fp = fopen(path, "w");

	if (!fp) {
		fprintf(stderr, "cannot write %s\n", path);
		return -1;
	}
	fputs(body, fp);
	fclose(fp);
	return 0;
}

/* log one message per level through a freshly initialised zlog */
static int log_three(const char *conf)
{
	zlog_category_t *c;

	if (zlog_init(conf)) {
		fprintf(stderr, "zlog_init(%s) fail\n", conf);
		return -1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		fprintf(stderr, "zlog_get_category fail\n");
		zlog_fini();
		return -1;
	}

	zlog_debug(c, "debug-msg");
	zlog_info(c, "info-msg");
	zlog_error(c, "error-msg");

	zlog_fini();
	return 0;
}

static int slurp(const char *path, char *buf, size_t size)
{
	FILE *fp = fopen(path, "r");
	size_t len;

	memset(buf, 0x00, size);
	if (!fp)                    /* nothing logged at all */
		return 0;

	len = fread(buf, 1, size - 1, fp);
	buf[len] = '\0';
	fclose(fp);
	return 0;
}

#define CONF_LEVEL(writer, out) \
	"[global]\n" \
	"use_writer_thread = " writer "\n" \
	"fifo_size = 1mb\n" \
	"[formats]\n" \
	"simple = \"%m%n\"\n" \
	"[rules]\n" \
	"my_cat.INFO    \"" out "\"; simple\n"

static int test_level_filtered_rule(void)
{
	char with_writer[256];
	char without_writer[256];

	remove("consumer_rules_async.txt");
	remove("consumer_rules_sync.txt");

	if (write_conf("consumer_rules_async.conf",
			CONF_LEVEL("1", "consumer_rules_async.txt")))
		return -1;
	if (write_conf("consumer_rules_sync.conf",
			CONF_LEVEL("0", "consumer_rules_sync.txt")))
		return -1;

	if (log_three("consumer_rules_async.conf"))
		return -1;
	if (log_three("consumer_rules_sync.conf"))
		return -1;

	slurp("consumer_rules_async.txt", with_writer, sizeof(with_writer));
	slurp("consumer_rules_sync.txt", without_writer, sizeof(without_writer));

	/* my_cat.INFO takes info and above, and drops debug */
	if (strcmp(without_writer, "info-msg\nerror-msg\n")) {
		fprintf(stderr, "without the writer thread, got [%s]\n", without_writer);
		return -1;
	}

	if (strcmp(with_writer, without_writer)) {
		fprintf(stderr, "the writer thread changed what a level filtered rule "
				"selects: got [%s], want [%s]\n", with_writer, without_writer);
		return -1;
	}

	return 0;
}

/* the consumer has no thread of its own to hand to an output */
static int test_syslog_rule(void)
{
	if (write_conf("consumer_rules_syslog.conf",
			"[global]\n"
			"use_writer_thread = 1\n"
			"fifo_size = 1mb\n"
			"[rules]\n"
			"my_cat.*    >syslog, LOG_USER\n"))
		return -1;

	return log_three("consumer_rules_syslog.conf");
}

static int test_rotate_with_archive_specs(void)
{
	zlog_category_t *c;
	int i;

	remove("consumer_rules_rotate.log");

	if (write_conf("consumer_rules_rotate.conf",
			"[global]\n"
			"use_writer_thread = 1\n"
			"fifo_size = 1mb\n"
			"[formats]\n"
			"simple = \"%m%n\"\n"
			"[rules]\n"
			"my_cat.*    \"consumer_rules_rotate.log\", 1K * 3 ~ "
			"\"consumer_rules_rotate-%d(%Y%m%d).#2s.log\"; simple\n"))
		return -1;

	if (zlog_init("consumer_rules_rotate.conf")) {
		fprintf(stderr, "zlog_init fail\n");
		return -1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		zlog_fini();
		return -1;
	}

	/* enough to cross 1K and reach the archive path */
	for (i = 0; i < 200; i++)
		zlog_info(c, "0123456789012345678901234567890123456789 %d", i);

	zlog_fini();
	return 0;
}

int main(void)
{
	if (test_level_filtered_rule()) {
		fprintf(stderr, "test_level_filtered_rule fail\n");
		return 1;
	}

	if (test_syslog_rule()) {
		fprintf(stderr, "test_syslog_rule fail\n");
		return 2;
	}

	if (test_rotate_with_archive_specs()) {
		fprintf(stderr, "test_rotate_with_archive_specs fail\n");
		return 3;
	}

	printf("consumer rule tests passed\n");
	return 0;
}
