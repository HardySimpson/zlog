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
#include "errno.h"
#include "zc_defs.h"
#include "record.h"

void zlog_record_profile(zlog_record_t *a_record, int flag)
{
	zc_assert(a_record,);
	zc_profile(flag, "--record:[%p][%s:%p]--", a_record, a_record->name,  a_record->output);
	return;
}

void zlog_record_del(zlog_record_t *a_record)
{
	zc_assert(a_record,);
	free(a_record);
	zc_debug("zlog_record_del[%p]", a_record);
	return;
}

zlog_record_t *zlog_record_new(const char *name, zlog_record_fn output)
{
	zlog_record_t *a_record;

	zc_assert(name, NULL);
	zc_assert(output, NULL);

	a_record = calloc(1, sizeof(zlog_record_t));
	if (!a_record) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	if (strlen(name) > sizeof(a_record->name) - 1) {
		zc_error("name[%s] is too long", name);
		goto err;
	}

	strcpy(a_record->name, name);
	a_record->output = output;

	zlog_record_profile(a_record, ZC_DEBUG);
	return a_record;
err:
	zlog_record_del(a_record);
	return NULL;
}
