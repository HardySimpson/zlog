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

#ifndef __zlog_buf_h
#define __zlog_buf_h

/* buf, is a dynamic expand buffer for one single log,
 * as one single log will interlace if use multiple write() to file.
 * and buf is always keep in a thread, to make each thread has its
 * own buffer to avoid lock.
 */

#include <stdarg.h>
#include <stdint.h>

typedef struct zlog_buf_s {
	char *start;
	char *tail;
	char *end;
	char *end_plus_1;

	size_t size_min;
	size_t size_max;
	size_t size_real;

	char truncate_str[MAXLEN_PATH + 1];
	size_t truncate_str_len;
} zlog_buf_t;


zlog_buf_t *zlog_buf_new(size_t min, size_t max, const char *truncate_str);
void zlog_buf_del(zlog_buf_t * a_buf);
void zlog_buf_profile(zlog_buf_t * a_buf, int flag);

int zlog_buf_vprintf(zlog_buf_t * a_buf, const char *format, va_list args);
int zlog_buf_append(zlog_buf_t * a_buf, const char *str, size_t str_len);
int zlog_buf_adjust_append(zlog_buf_t * a_buf, const char *str, size_t str_len,
			int left_adjust, size_t in_width, size_t out_width);
int zlog_buf_printf_dec32(zlog_buf_t * a_buf, uint32_t ui32, int width);
int zlog_buf_printf_dec64(zlog_buf_t * a_buf, uint64_t ui64, int width);
int zlog_buf_printf_hex(zlog_buf_t * a_buf, uint32_t ui32, int width);

#define zlog_buf_restart(a_buf) do { \
	a_buf->tail = a_buf->start; \
} while(0)

#define zlog_buf_len(a_buf) (a_buf->tail - a_buf->start)
#define zlog_buf_str(a_buf) (a_buf->start)
#define zlog_buf_seal(a_buf) do {*(a_buf)->tail = '\0';} while (0)

#endif
