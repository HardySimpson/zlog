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
#include "zc_defs.h"
#include "buf.h"

int main(int argc, char** argv)
{
	zlog_buf_t *a_buf;
	char *aa;

	a_buf = zlog_buf_new(10, 20, "ABC");
	if (!a_buf) {
		zc_error("zlog_buf_new fail");
		return -1;
	}

	aa = "123456789";
	zlog_buf_append(a_buf, aa, strlen(aa));
	zc_error("a_buf->start[%s]", a_buf->start);
	fwrite(a_buf->start, zlog_buf_len(a_buf), 1, stdout);
	zc_error("------------");

	aa = "0";
	zlog_buf_append(a_buf, aa, strlen(aa));
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	aa = "12345";
	zlog_buf_append(a_buf, aa, strlen(aa));
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	aa = "6789";
	zlog_buf_append(a_buf, aa, strlen(aa));
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	aa = "0";
	zlog_buf_append(a_buf, aa, strlen(aa));
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	aa = "22345";
	zlog_buf_append(a_buf, aa, strlen(aa));
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");


	aa = "abc";
	int i,j;
	for (i = 0; i <= 5; i++) {
		for (j = 0; j <= 5; j++) {
			zlog_buf_restart(a_buf);
			zc_error("left[1],max[%d],min[%d]", i, j);
			zlog_buf_adjust_append(a_buf, aa, strlen(aa), 1, 0, i, j);
			zc_error("a_buf->start[%s]", a_buf->start);

			zc_error("-----");

			zlog_buf_restart(a_buf);
			zc_error("left[0],max[%d],min[%d]", i, j);
			zlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, i, j);
			zc_error("a_buf->start[%s]", a_buf->start);
			zc_error("------------");
		}
	}

	aa = "1234567890";
	zc_error("left[0],max[%d],min[%d]", 15, 5);
	zlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, 15, 5);
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	aa = "1234567890";
	zlog_buf_restart(a_buf);
	zc_error("left[0],max[%d],min[%d]", 25, 5);
	zlog_buf_adjust_append(a_buf, aa, strlen(aa), 1, 0, 25, 5);
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	zlog_buf_restart(a_buf);
	zc_error("left[0],max[%d],min[%d]", 19, 5);
	zlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, 19, 5);
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	zlog_buf_restart(a_buf);
	zc_error("left[0],max[%d],min[%d]", 20, 5);
	zlog_buf_adjust_append(a_buf, aa, strlen(aa), 0, 0, 20, 5);
	zc_error("a_buf->start[%s]", a_buf->start);
	zc_error("------------");

	zlog_buf_del(a_buf);

	return 0;
}
