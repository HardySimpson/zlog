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

#ifndef __xlog_conf_h
#define  __xlog_conf_h

/**
 * @file conf.h
 * @brief xlog configure class
 */

#include "zc_defs.h"

/**
 * xlog conf struct
 */
typedef struct {
	char file[MAXLEN_PATH + 1];	/**< configure file path */
	char mtime[20 + 1];		/**< the last modified time of configure file */

	int init_chk_conf;		/**< check configure file flag */
	size_t buf_size_min;		/**< each buffer size min */
	size_t buf_size_max;		/**< each buffer size max */
	char rotate_lock_file[MAXLEN_PATH + 1];
					/**< global lock file for rotate,
					 *   xlog will create the file,
					 *   make sure your program has permission
					 *   to create and read-write the file.
					 *   Besides, if programs run by different users
					 *   who need to write and rotate the same log file,
					 *   make sure that each program has permission
					 *   to create and read-write the file.
					 */
	zc_arraylist_t *formats;	/**< list of all formats */
	zc_arraylist_t *rules;		/**< list of all rules */
} xlog_conf_t;

/**
 * xlog conf initer, read conf_file and fill a_conf
 *
 * @param a_conf the address pointed to by xlog_conf_t
 * @param conf_file configure file path. If it is NULL, read from XLOG_CONF_PATH or /etc/xlog.conf
 * @returns 0 for success, -1 for fail
 */
int xlog_conf_init(xlog_conf_t * a_conf, char *conf_file);

/**
 * xlog conf finisher, destroy formats list and rules list, clean all memeber of a_conf
 *
 * @param a_conf the address pointed to by xlog_conf_t
 */
void xlog_conf_fini(xlog_conf_t * a_conf);

/**
 * Update a_conf from conf_file.
 *
 * @param a_conf the address pointed to by xlog_conf_t
 * @param conf_file configure file path. If it is NULL, read from last conf_file
 */
int xlog_conf_update(xlog_conf_t * a_conf, char *conf_file);

/**
 * Output detail of xlog_conf_t to XLOG_ERROR_LOG.
 *
 * @param a_conf the address pointed to by xlog_conf_t.
 */
void xlog_conf_profile(xlog_conf_t * a_conf);

#endif
