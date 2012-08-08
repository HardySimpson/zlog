/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
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
