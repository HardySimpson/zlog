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

#include "zc_profile.c"
#include "zc_hashtable.h"
#include "zc_hashtable.c"

void myfree(void *kv)
{
}

int main(void)
{
	zc_hashtable_t *a_table;
	zc_hashtable_entry_t *a_entry;

	a_table = zc_hashtable_new(20,
		zc_hashtable_str_hash,
		zc_hashtable_str_equal,
		myfree, myfree);

	zc_hashtable_put(a_table, "aaa", "bnbb");
	zc_hashtable_put(a_table, "bbb", "bnbb");
	zc_hashtable_put(a_table, "ccc", "bnbb");

	zc_hashtable_put(a_table, "aaa", "123");

	zc_hashtable_foreach(a_table, a_entry) {
		printf("k[%s],v[%s]\n", (char*)a_entry->key, (char*)a_entry->value);
	}

	printf("getv[%s]\n", (char*)zc_hashtable_get(a_table, "ccc"));

	zc_hashtable_remove(a_table, "ccc");

	zc_hashtable_foreach(a_table, a_entry) {
		printf("k[%s],v[%s]\n", (char*)a_entry->key, (char*)a_entry->value);
	}


	zc_hashtable_remove(a_table, NULL);
	zc_hashtable_del(NULL);

	zc_hashtable_del(a_table);
	return 0;
}

