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

#ifndef __zc_arraylist_h
#define __zc_arraylist_h

#define ARRAY_LIST_DEFAULT_SIZE 32

typedef void (*zc_arraylist_del_fn) (void *data);
typedef int (*zc_arraylist_cmp_fn) (void *data1, void *data2);

/* make zc_arraylist_foreach speed up, so keep struct defination here */
typedef struct {
	void **array;
	int len;
	int size;
	zc_arraylist_del_fn del;
} zc_arraylist_t;

zc_arraylist_t *zc_arraylist_new(zc_arraylist_del_fn del);
void zc_arraylist_del(zc_arraylist_t * a_list);

int zc_arraylist_set(zc_arraylist_t * a_list, int i, void *data);
int zc_arraylist_add(zc_arraylist_t * a_list, void *data);
int zc_arraylist_sortadd(zc_arraylist_t * a_list, zc_arraylist_cmp_fn cmp,
			 void *data);

#define zc_arraylist_len(a_list)  (a_list->len)

#define zc_arraylist_get(a_list, i) \
	 ((i >= a_list->len) ? NULL : a_list->array[i])

#define zc_arraylist_foreach(a_list, i, a_unit) \
	for(i = 0, a_unit = a_list->array[0]; (i < a_list->len) && (a_unit = a_list->array[i], 1) ; i++)

#endif
