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
	
	zlog_category_t *zc;

	rc = zlog_init("test_init.conf");
	if (rc) {
		printf("init fail");
		return -2;
	}
	zc = zlog_get_category("my_cat");
	if (!zc) {
		printf("zlog_get_category fail\n");
		zlog_fini();
		return -1;
	}
	zlog_info(zc, "before update");
	sleep(1);
	rc = zlog_reload("test_init.2.conf");
	if (rc) {
		printf("update fail\n");
	}
	zlog_info(zc, "after update");
	zlog_profile();
	zlog_fini();

	sleep(1);
	zlog_init("test_init.conf");
	zc = zlog_get_category("my_cat");
	if (!zc) {
		printf("zlog_get_category fail\n");
		zlog_fini();
		return -1;
	}
	zlog_info(zc, "init again");
	zlog_fini();
	
	return 0;
}
