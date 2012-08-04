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
#include "rotater.h"

typedef struct zlog_conf_s {
	char file[MAXLEN_PATH + 1];
	char mtime[20 + 1];

	int strict_init;
	size_t buf_size_min;
	size_t buf_size_max;

	char rotate_lock_file[MAXLEN_CFG_LINE + 1];
	zlog_rotater_t *rotater;

	char default_format_line[MAXLEN_CFG_LINE + 1];
	zlog_format_t *default_format;

	unsigned int file_perms;
	size_t fsync_period;
	size_t reload_conf_period;

	zc_arraylist_t *levels;
	zc_arraylist_t *formats;
	zc_arraylist_t *rules;
} zlog_conf_t;

extern zlog_conf_t * zlog_env_conf;

zlog_conf_t *zlog_conf_new(const char *confpath);
void zlog_conf_del(zlog_conf_t * a_conf);
void zlog_conf_profile(zlog_conf_t * a_conf, int flag);

#endif
