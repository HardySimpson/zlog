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
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "zlog.h"

int main(int argc, char** argv)
{
	int rc;
	int k;
	int i;

	if (argc != 2) {
		printf("test_leak ntime\n");
		return -1;
	}

	rc = zlog_init("test_leak.conf");

	k = atoi(argv[1]);
	while (k-- > 0) {
		i = rand();
		switch (i % 4) {
		case 0:
			rc = dzlog_init("test_leak.conf", "xxx");
			dzlog_info("init, rc=[%d]", rc);
			break;
		case 1:
			rc = zlog_reload(NULL);
			dzlog_info("reload null, rc=[%d]", rc);
			break;
		case 2:
			rc = zlog_reload("test_leak.2.conf");
			dzlog_info("reload 2, rc=[%d]", rc);
			break;
		case 3:
			zlog_fini();
			printf("fini\n");
	//		printf("zlog_finish\tj=[%d], rc=[%d]\n", j, rc);
			break;
		}
	}

	zlog_fini();
	return 0;
}
