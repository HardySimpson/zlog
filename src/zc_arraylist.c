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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "zc_defs.h"

zc_arraylist_t *zc_arraylist_new(zc_arraylist_del_fn del)
{
	zc_arraylist_t *a_list;

	a_list = (zc_arraylist_t *) calloc(1, sizeof(zc_arraylist_t));
	if (!a_list) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}
	a_list->size = ARRAY_LIST_DEFAULT_SIZE;
	a_list->len = 0;

	/* this could be NULL */
	a_list->del = del;
	a_list->array = (void **)calloc(a_list->size, sizeof(void *));
	if (!a_list->array) {
		zc_error("calloc fail, errno[%d]", errno);
		free(a_list);
		return NULL;
	}

	return a_list;
}

void zc_arraylist_del(zc_arraylist_t * a_list)
{
	int i;

	if (!a_list)
		return;
	if (a_list->del) {
		for (i = 0; i < a_list->len; i++) {
			if (a_list->array[i])
				a_list->del(a_list->array[i]);
		}
	}
	if (a_list->array)
		free(a_list->array);
	free(a_list);
	return;
}

static int zc_arraylist_expand_inner(zc_arraylist_t * a_list, int max)
{
	void *tmp;
	int new_size;
	int diff_size;

	new_size = zc_max(a_list->size * 2, max);
	tmp = realloc(a_list->array, new_size * sizeof(void *));
	if (!tmp) {
		zc_error("realloc fail, errno[%d]", errno);
		return -1;
	}
	a_list->array = (void **)tmp;
	diff_size = new_size - a_list->size;
	if (diff_size) memset(a_list->array + a_list->size, 0x00, diff_size * sizeof(void *));
	a_list->size = new_size;
	return 0;
}

int zc_arraylist_set(zc_arraylist_t * a_list, int idx, void *data)
{
	if (idx > a_list->size - 1) {
		if (zc_arraylist_expand_inner(a_list, idx)) {
			zc_error("expand_internal fail");
			return -1;
		}
	}
	if (a_list->array[idx] && a_list->del) a_list->del(a_list->array[idx]);
	a_list->array[idx] = data;
	if (a_list->len <= idx)
		a_list->len = idx + 1;
	return 0;
}

int zc_arraylist_add(zc_arraylist_t * a_list, void *data)
{
	return zc_arraylist_set(a_list, a_list->len, data);
}

/* assum idx < len */
static int zc_arraylist_insert_inner(zc_arraylist_t * a_list, int idx,
				     void *data)
{
	if (a_list->array[idx] == NULL) {
		a_list->array[idx] = data;
		return 0;
	}
	if (a_list->len > a_list->size - 1) {
		if (zc_arraylist_expand_inner(a_list, 0)) {
			zc_error("expand_internal fail");
			return -1;
		}
	}
	memmove(a_list->array + idx + 1, a_list->array + idx,
		(a_list->len - idx) * sizeof(void *));
	a_list->array[idx] = data;
	a_list->len++;
	return 0;
}

int zc_arraylist_sortadd(zc_arraylist_t * a_list, zc_arraylist_cmp_fn cmp,
			 void *data)
{
	int i;

	for (i = 0; i < a_list->len; i++) {
		if ((*cmp) (a_list->array[i], data) > 0)
			break;
	}

	if (i == a_list->len)
		return zc_arraylist_add(a_list, data);
	else
		return zc_arraylist_insert_inner(a_list, i, data);
}
