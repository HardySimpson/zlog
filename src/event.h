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

#ifndef __zlog_event_h
#define __zlog_event_h

typedef enum {
	ZLOG_FMT = 0,
	ZLOG_HEX = 1,
} zlog_event_cmd;

typedef struct zlog_time_cache_s {
	zc_sds str;
	time_t sec;
} zlog_time_cache_t;

typedef struct {
	zc_sds category_name; /* just a pointer to the real category's name */
	const char *file;
	size_t file_len;
	const char *func;
	size_t func_len;
	long line;
	int level;

	const void *hex_buf;
	size_t hex_buf_len;
	const char *str_format;
	va_list str_args;
	zlog_event_cmd generate_cmd;

	struct timeval time_stamp;

	time_t time_local_sec; 
	struct tm time_local; /*a cache, time_local == localtime(time_local_sec); */

	zlog_time_cache_t *time_caches;
	int time_cache_count;

	zc_sds host_name;

	pid_t pid;
	pid_t last_pid;
	zc_sds pid_str; /* a cache, pit_str == printf("%d", last_pid) */

	pthread_t tid;
	zc_sds tid_str;
	zc_sds tid_hex_str;
} zlog_event_t;


zlog_event_t *zlog_event_new(int time_cache_count);
void zlog_event_del(zlog_event_t * a_event);
void zlog_event_profile(zlog_event_t * a_event, int flag);

void zlog_event_set_fmt(zlog_event_t * a_event, zc_sds category_name,
			const char *file, size_t file_len, const char *func, size_t func_len, long line, int level,
			const char *str_format, va_list str_args);

void zlog_event_set_hex(zlog_event_t * a_event, zc_sds category_name,
			const char *file, size_t file_len, const char *func, size_t func_len, long line, int level,
			const void *hex_buf, size_t hex_buf_len);

#endif
