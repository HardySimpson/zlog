/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
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

#ifndef __xlog_mdc_h
#define __xlog_mdc_h

/**
 * @file mdc.h
 * @brief xlog mapped diagnostic contexts
 */

#include "zc_defs.h"

typedef struct {
	zc_hashtable_t *tab;
} xlog_mdc_t;

/**
 * constructor
 */
xlog_mdc_t *xlog_mdc_new(void);

/**
 * destructor
 */
void xlog_mdc_del(xlog_mdc_t *a_mdc);

/**
 * remove all values from mdc, remain map itself
 */
void xlog_mdc_clean(xlog_mdc_t *a_mdc);

/**
 * put key-value into a_mdc
 */
int xlog_mdc_put(xlog_mdc_t *a_mdc, char *key, char *value);

/**
 * get value from a_mdc
 */
char *xlog_mdc_get(xlog_mdc_t *a_mdc, char *key);

/**
 * remove key-value from a_mdc
 */
void xlog_mdc_remove(xlog_mdc_t *a_mdc, char *key);

typedef struct xlog_mdc_kv_t {
	char key[MAXLEN_PATH + 1];
	char value[MAXLEN_PATH + 1];
	size_t value_len;
} xlog_mdc_kv_t;
/**
 * get xlog_mdc_kv_t entry from a_mdc
 */
xlog_mdc_kv_t *xlog_mdc_get_kv(xlog_mdc_t *a_mdc, char *key);

#endif
