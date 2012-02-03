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
#define  __zlog_conf_h

/**
 * @file conf.h
 * @brief configure file representation
 */

#include "zc_defs.h"

typedef struct {
	char file[MAXLEN_PATH + 1];	/**< configure file path */
	char mtime[20 + 1];		/**< the last modified time of configure file */

	int ignore_error_format_rule;	/**< if format or rule syntax is error, omit it, default 0 */
	size_t buf_size_min;		/**< each buffer size min */
	size_t buf_size_max;		/**< each buffer size max */
	char rotate_lock_file[MAXLEN_PATH + 1];
					/**< global lock file for rotate,
					 *   zlog will create the file,
					 *   make sure your program has permission
					 *   to create and read-write the file.
					 *   Besides, if programs run by different users
					 *   who need to write and rotate the same log file,
					 *   make sure that each program has permission
					 *   to create and read-write the file.
					 */
	zc_arraylist_t *formats;	/**< list of all formats */
	zc_arraylist_t *rules;		/**< list of all rules */
} zlog_conf_t;

int zlog_conf_init(zlog_conf_t * a_conf, char *conf_file);

void zlog_conf_fini(zlog_conf_t * a_conf);

int zlog_conf_update(zlog_conf_t * a_conf, char *conf_file);

void zlog_conf_profile(zlog_conf_t * a_conf);

#endif
