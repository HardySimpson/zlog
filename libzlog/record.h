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

#ifndef __zlog_record_h
#define __zlog_record_h

#include "zc_defs.h"

/* record is user-defined output function and it's name from configure file */
typedef struct zlog_msg_s {
	char *buf;
	size_t len;
	char *path;
} zlog_msg_t; /* 3 of this first, see need thread or not later */

typedef int (*zlog_record_fn)(zlog_msg_t * msg);

typedef struct zlog_record_s {
	char name[MAXLEN_PATH + 1];
	zlog_record_fn output;
} zlog_record_t;

zlog_record_t *zlog_record_new(const char *name, zlog_record_fn output);
void zlog_record_del(zlog_record_t *a_record);
void zlog_record_profile(zlog_record_t *a_record, int flag);

#endif
