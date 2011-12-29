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

#ifndef __xlog_buf_h
#define __xlog_buf_h

#include <stdarg.h>

/**
 * @file buf.h
 * @brief a self expand buffer class.
 */

/**
 * xlog buffer struct
 */
typedef struct {
	size_t size_min;		/**< default 1024 */
	size_t size_max;		/**< 0 means unlimit */
	size_t size_step;		/**< if size_max != 0 and need to expand,
					 *   increate size_step every time.
					 */

	char truncate_str[MAXLEN_PATH + 1];  /**< if buf is full, the end of buf will
					      *   be filled with truncate_str
					      */
	size_t truncate_str_len;	/**< strlen(truncate_str) */

	size_t size_real;		/**< real buffer size now */
	char *start;			/**< buffer start pointer */
	char *end;			/**< buffer end pointer */
} xlog_buf_t;

/**
 * xlog_buf_t constructor
 * 
 * @param bufsize_min if is 0, set inner size_min = 1024.
 * @param bufsize_max if is 0, means buffer can expand unlimited.
 * @param truncate_str if buffer is full, the end of buf will be filled with truncate_str.
 * @returns xlog_buf_t pointer for success, NULL for fail.
 */
xlog_buf_t *xlog_buf_new(size_t bufsize_min, size_t bufsize_max, const char *truncate_str);

/**
 * xlog_buf_t destructor
 * 
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 */
void xlog_buf_del(xlog_buf_t * a_buf);

/**
 * set buffer to '\\0', from start to end, and restart the buffer
 * 
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 */
void xlog_buf_clean(xlog_buf_t * a_buf);

/**
 * set end = start, so next write buffer function can write from start.
 * 
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 */
void xlog_buf_restart(xlog_buf_t * a_buf);

/**
 * buffer write function, printf version
 *
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 * @param format C printf style format.
 * @returns 0 for success, -1 for fail, 1 for buffer is full(expand to size_max).
 */
int xlog_buf_printf(xlog_buf_t * a_buf, const char *format, ...);

/**
 * buffer write function, vprintf version
 *
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 * @param format C printf style format.
 * @param args a va_list, will be va_copy() inner.
 * @returns 0 for success, -1 for fail, 1 for buffer is full(expand to size_max).
 */
int xlog_buf_vprintf(xlog_buf_t * a_buf, const char *format, va_list args);

/**
 * buffer write function, append version
 *
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 * @param str string wanna to be append at the tail of buffer.
 * @param str_len strlen(str)
 * @returns 0 for success, -1 for fail, 1 for buffer is full(expand to size_max).
 */
int xlog_buf_append(xlog_buf_t * a_buf, const char *str, size_t str_len);

/**
 * buffer write function, strftime version
 *
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 * @param time_fmt strftime's format.
 * @param time_len Guess the length after strftime, will be used for buffer expand inner.
 * @param tm strftime's tm.
 * @returns 0 for success, -1 for fail, 1 for buffer is full(expand to size_max).
 */
int xlog_buf_strftime(xlog_buf_t * a_buf, const char *time_fmt, size_t time_len,
			const struct tm *a_tm);

/**
 * Output detail of xlog_buf_t to XLOG_ERROR_LOG.
 *
 * @param a_buf xlog_buf_t pointer, shall not be NULL.
 */
void xlog_buf_profile(xlog_buf_t * a_buf);

#endif
