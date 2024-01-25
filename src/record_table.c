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

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "zc_defs.h"
#include "record_table.h"

void zlog_record_table_profile(zc_hashtable_t * records, int flag)
{
	zc_hashtable_entry_t *a_entry;
	zlog_record_t *a_record;

	zc_assert(records,);
	zc_profile(flag, "-record_table[%p]-", records);
	zc_hashtable_foreach(records, a_entry) {
		a_record = (zlog_record_t *) a_entry->value;
		zlog_record_profile(a_record, flag);
	}
	return;
}

/*******************************************************************************/

void zlog_record_table_del(zc_hashtable_t * records)
{
	zc_assert(records,);
	zc_hashtable_del(records);
	zc_debug("zlog_record_table_del[%p]", records);
	return;
}

zc_hashtable_t *zlog_record_table_new(void)
{
	zc_hashtable_t *records;

	records = zc_hashtable_new(20,
			 (zc_hashtable_hash_fn) zc_hashtable_str_hash,
			 (zc_hashtable_equal_fn) zc_hashtable_str_equal,
			 NULL, (zc_hashtable_del_fn) zlog_record_del);
	if (!records) {
		zc_error("zc_hashtable_new fail");
		return NULL;
	} else {
		zlog_record_table_profile(records, ZC_DEBUG);
		return records;
	}
}
/*******************************************************************************/
