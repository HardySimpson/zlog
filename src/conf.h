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

typedef struct zlog_conf_s {
	zc_sds file;
	long mtime;
	int strict_init;
	long auto_reload;

	zc_sds rotate_lock_file;
	zc_sds default_deepness_str;
	zc_sds default_format_str;

	zlog_rotater_t *rotater;
	zlog_deepness_t *default_deepness;
	zlog_format_t *default_format;

	zc_arraylist_t *deepness;
	zc_arraylist_t *levels;
	zc_arraylist_t *formats;
	zc_arraylist_t *rules;
} zlog_conf_t;

extern zlog_conf_t * zlog_env_conf;

zlog_conf_t *zlog_conf_new(const char *confpath);
void zlog_conf_del(zlog_conf_t * a_conf);
void zlog_conf_profile(zlog_conf_t * a_conf, int flag);

#endif
