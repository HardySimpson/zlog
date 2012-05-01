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

#include <stdarg.h>

typedef struct zlog_buf_s {
	size_t size_min;
	size_t size_max;
	size_t size_step;
	char truncate_str[MAXLEN_PATH + 1];
	size_t truncate_str_len;
	size_t size_real;
	char *start;
	char *end;
} zlog_buf_t;

zlog_buf_t *zlog_buf_new(size_t buf_size_min, size_t buf_size_max,
			 const char *truncate_str);
void zlog_buf_del(zlog_buf_t * a_buf);
void zlog_buf_profile(zlog_buf_t * a_buf, int flag);

void zlog_buf_restart(zlog_buf_t * a_buf);
int zlog_buf_printf(zlog_buf_t * a_buf, const char *format, ...);
int zlog_buf_vprintf(zlog_buf_t * a_buf, const char *format, va_list args);
int zlog_buf_append(zlog_buf_t * a_buf, const char *str, size_t str_len);
int zlog_buf_strftime(zlog_buf_t * a_buf, const char *time_fmt, size_t time_len,
		      const struct tm *a_tm);

#endif
