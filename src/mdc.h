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

#ifndef __zlog_mdc_h
#define __zlog_mdc_h

#include "zc_defs.h"

typedef struct zlog_mdc_s zlog_mdc_t;
struct zlog_mdc_s {
	zc_hashtable_t *tab;
};

zlog_mdc_t *zlog_mdc_new(void);
void zlog_mdc_del(zlog_mdc_t * a_mdc);
void zlog_mdc_profile(zlog_mdc_t *a_mdc, int flag);

void zlog_mdc_clean(zlog_mdc_t * a_mdc);
int zlog_mdc_put(zlog_mdc_t * a_mdc, const char *key, const char *value);
char *zlog_mdc_get(zlog_mdc_t * a_mdc, const char *key);
void zlog_mdc_remove(zlog_mdc_t * a_mdc, const char *key);

typedef struct zlog_mdc_kv_s {
	char key[MAXLEN_PATH + 1];
	char value[MAXLEN_PATH + 1];
	size_t value_len;
} zlog_mdc_kv_t;

zlog_mdc_kv_t *zlog_mdc_get_kv(zlog_mdc_t * a_mdc, const char *key);

#endif
