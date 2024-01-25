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
#include <stdlib.h>
#include "zlog.h"

int main(int argc, char** argv)
{
	unsigned char aa[32];
	int i, j;

	if (argc != 3) {
		printf("useage: test_bitmap i j\n");
		exit(1);
	}

	dzlog_init(NULL, "AA");


	i = atoi(argv[1]);
	j = atoi(argv[2]);

	memset(aa, 0x00, sizeof(aa));

	/* 32 byte, 256 bit
	 * [11111..1100...00]
	 *          i
	 */
	aa[i/8] |=  ~(0xFF << (8 - i % 8));
	memset(aa + i/8 + 1, 0xFF, sizeof(aa) - i/8 - 1);

	hdzlog_info(aa, sizeof(aa));

	dzlog_info("%0x", aa[j/8]);
	dzlog_info("%0x", aa[j/8] >> 6);

	/* see j of bits fits */
	dzlog_info("%0x", ~((aa[j/8] >> (7 - j % 8)) & 0x01) );

	zlog_fini();
	
	return 0;
}
