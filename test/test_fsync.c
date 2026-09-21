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

#include <stdio.h>
#include <string.h>

#include "zlog.h"

#define LOG_FILE	"fsync.log"
#define MARK		"the log has to be on disk by now"

static int log_file_has_mark(void)
{
	FILE	*fp;
	char	line[1024];
	int	found = 0;

	fp = fopen(LOG_FILE, "r");
	if (!fp) {
		printf("fopen[%s] fail\n", LOG_FILE);
		return 0;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (strstr(line, MARK)) {
			found = 1;
			break;
		}
	}

	fclose(fp);

	return found;
}

int main(int argc, char** argv)
{
	int rc;
	int i;
	zlog_category_t *zc;

	remove(LOG_FILE);

	if (zlog_fsync() == 0) {
		printf("zlog_fsync before init should fail\n");
		return -1;
	}

	rc = zlog_init("test_fsync.conf");
	if (rc) {
		printf("init failed\n");
		return -2;
	}

	zc = zlog_get_category("my_cat");
	if (!zc) {
		printf("get cat fail\n");
		zlog_fini();
		return -3;
	}

	/* enough of a backlog that the writer thread can not have caught
	 * up with it by itself */
	for (i = 0; i < 10000; i++) {
		zlog_info(zc, "filler %d", i);
	}
	zlog_info(zc, MARK);

	rc = zlog_fsync();
	if (rc) {
		printf("zlog_fsync fail, rc[%d]\n", rc);
		zlog_fini();
		return -4;
	}

	/* the writer thread is still running, so this only holds because
	 * zlog_fsync() drained its queue before syncing */
	if (!log_file_has_mark()) {
		printf("[%s] not in [%s] after zlog_fsync\n", MARK, LOG_FILE);
		zlog_fini();
		return -5;
	}

	zlog_fini();

	return 0;
}
