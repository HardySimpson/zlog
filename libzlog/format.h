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

#ifndef __zlog_format_h
#define __zlog_format_h

/**
 * @file format.h
 * @brief log pattern class
 */

#include "thread.h"
#include "zc_defs.h"

/**
 * zlog format struct
 */
typedef struct {
	char name[MAXLEN_CFG_LINE + 1];		/**< name of format */
	char pattern[MAXLEN_CFG_LINE + 1];	/**< pattern of format */
	zc_arraylist_t *pattern_specs;		/**< list of pattern's specifiers */
} zlog_format_t;

/**
 * zlog format constructor, will parses a conf file line to name and pattern,
 * then parse pattern to pattern_specs.
 *
 * @param line configure file line of format
 * @parma line_len strlen(line)
 * @returns zlog_format_t pointer for success, NULL for fail
 */
zlog_format_t *zlog_format_new(const char *line, long line_len);

/**
 * zlog format destructor
 *
 * @param zlog_format_t pointer
 */
void zlog_format_del(zlog_format_t * a_format);

/**
 * generate msg from one format.pattern_specs,
 * if buffer in a_thread is not long enough, truncate it
 *
 * @param a_format zlog_format_t pointer
 * @parma a_thread contains buf and event
 * @returns 0 for success(maybe truncated), -1 for fail
 */
int zlog_format_gen_msg(zlog_format_t * a_format, zlog_thread_t * a_thread);


/**
 * Output detail of zlog_format_t to ZLOG_ERROR_LOG.
 *
 * @param a_format zlog_format_t pointer, shall not be NULL.
 */
void zlog_format_profile(zlog_format_t * a_format);

#endif
