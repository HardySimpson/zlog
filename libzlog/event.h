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

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include "zc_defs.h"

typedef enum {
	ZLOG_FMT = 0,
	ZLOG_HEX = 1,
} zlog_event_cmd;

typedef struct {
	char *category_name;
	size_t *category_name_len;
	char host_name[256 + 1];
	size_t host_name_len;

	char *file;
	long line;
	int level;

	void *hex_buf;
	size_t hex_buf_len;
	char *str_format;
	va_list str_args;
	zlog_event_cmd generate_cmd;

	struct timeval time_stamp;
	struct tm local_time;	
	char us[6 + 1];
	char time_fmt_msus[MAXLEN_CFG_LINE + 1];

	pid_t pid;
	pthread_t tid;
} zlog_event_t;

zlog_event_t *zlog_event_new(void);
void zlog_event_del(zlog_event_t * a_event);
void zlog_event_profile(zlog_event_t * a_event, int flag);

void zlog_event_refresh(zlog_event_t * a_event,
			char *category_name, size_t * category_name_len,
			char *file, long line, int level,
			void *hex_buf, size_t hex_buf_len, char *str_format,
			va_list str_args, int generate_cmd);

#endif
