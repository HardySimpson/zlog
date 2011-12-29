/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __zc_arraylist_h
#define __zc_arraylist_h

#define ARRAY_LIST_DEFAULT_SIZE 32

typedef void (*zc_arraylist_del_fn) (void *data);
typedef int (*zc_arraylist_cmp_fn) (void *data1, void *data2);

typedef struct {
	void **array;
	int len;
	int size;
	zc_arraylist_del_fn del_fn;
} zc_arraylist_t;

zc_arraylist_t *zc_arraylist_new(zc_arraylist_del_fn del_fn);
void zc_arraylist_del(zc_arraylist_t * a_list);

void *zc_arraylist_get(zc_arraylist_t * a_list, int i);
int zc_arraylist_set(zc_arraylist_t * a_list, int i, void *data);
int zc_arraylist_add(zc_arraylist_t * a_list, void *data);
int zc_arraylist_sortadd(zc_arraylist_t * a_list, zc_arraylist_cmp_fn, void *data);

int zc_arraylist_len(zc_arraylist_t * a_list);

#endif
