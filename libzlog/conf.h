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

#ifndef __zlog_conf_h
#define __zlog_conf_h

#include "zc_defs.h"
#include "format.h"

typedef struct {
	char file[MAXLEN_PATH + 1];
	char mtime[20 + 1];

	int ignore_error_format_rule;
	size_t buf_size_min;
	size_t buf_size_max;
	char rotate_lock_file[MAXLEN_PATH + 1];
	zlog_format_t *default_format;
	zc_arraylist_t *formats;
	zc_arraylist_t *rules;
} zlog_conf_t;

int zlog_conf_init(zlog_conf_t * a_conf, char *conf_file);
void zlog_conf_fini(zlog_conf_t * a_conf);
int zlog_conf_update(zlog_conf_t * a_conf, char *conf_file);
void zlog_conf_profile(zlog_conf_t * a_conf);

#endif
