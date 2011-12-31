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

#ifndef __xlog_format_h
#define __xlog_format_h

/**
 * @file format.h
 * @brief log pattern class
 */

#include "thread.h"
#include "zc_defs.h"

/**
 * xlog format struct
 */
typedef struct {
	char name[MAXLEN_CFG_LINE + 1];		/**< name of format */
	char pattern[MAXLEN_CFG_LINE + 1];	/**< pattern of format */
	zc_arraylist_t *pattern_specs;		/**< list of pattern's specifiers */
} xlog_format_t;

/**
 * xlog format constructor, will parses a conf file line to name and pattern,
 * then parse pattern to pattern_specs.
 *
 * @param line configure file line of format
 * @parma line_len strlen(line)
 * @returns xlog_format_t pointer for success, NULL for fail
 */
xlog_format_t *xlog_format_new(const char *line, long line_len);

/**
 * xlog format destructor
 *
 * @param xlog_format_t pointer
 */
void xlog_format_del(xlog_format_t * a_format);

/**
 * generate msg from one format.pattern_specs,
 * if buffer in a_thread is not long enough, truncate it
 *
 * @param a_format xlog_format_t pointer
 * @parma a_thread contains buf and event
 * @returns 0 for success(maybe truncated), -1 for fail
 */
int xlog_format_gen_msg(xlog_format_t * a_format, xlog_thread_t * a_thread);


/**
 * Output detail of xlog_format_t to XLOG_ERROR_LOG.
 *
 * @param a_format xlog_format_t pointer, shall not be NULL.
 */
void xlog_format_profile(xlog_format_t * a_format);

#endif
