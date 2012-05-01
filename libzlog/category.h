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

#ifndef __zlog_category_h
#define __zlog_category_h

#include "zc_defs.h"
#include "thread.h"

typedef struct zlog_category_s {
	char name[MAXLEN_PATH + 1];
	size_t name_len;
	zc_arraylist_t *match_rules;
} zlog_category_t;

zlog_category_t *zlog_category_new(char *name, zc_arraylist_t * rules);
void zlog_category_del(zlog_category_t * a_category);
void zlog_category_profile(zlog_category_t *a_category, int flag);

int zlog_category_output(zlog_category_t * a_cat, zlog_thread_t * a_thread);
int zlog_category_obtain_rules(zlog_category_t * a_category, zc_arraylist_t * rules);

#endif
