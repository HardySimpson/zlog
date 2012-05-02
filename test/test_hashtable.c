/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The zlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The zlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the zlog Library. If not, see <http://www.gnu.org/licenses/>.
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

