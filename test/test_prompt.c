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
#include "zlog.h"
#include <unistd.h>

int main(int argc, char** argv)
{
	int rc;
	zlog_category_t *zc,*pzc;

	rc = zlog_init("test_prompt.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	zc = zlog_get_category("my_cat");
	pzc = zlog_get_category("prompt");
	if (!zc || !pzc) {
		printf("get cat fail\n");
		zlog_fini();
		return -2;
	}

	zlog_debug(zc, "%s%d", "hello, zlog ", 1);
	zlog_info(zc, "hello, zlog 2");

	for (int i =0; i<15;i++){
		zlog_info(pzc, "prompt>");
        usleep(300000);
        if (!(i % 3))
            zlog_debug(zc, "dummy log entry %d", i);
        if (!(i % 5))
            zlog_info(zc, "hello, zlog %d", i);
    }
    zlog_info(zc, "done");

    //	zlog_profile();

    zlog_fini();

    return 0;
}
