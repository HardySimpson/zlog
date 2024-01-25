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
	zc_debug("zlog_record_del[%p]", a_record);
    free(a_record);
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
