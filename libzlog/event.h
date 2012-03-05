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

/**
 * @file event.h
 * @brief keep all infomation from user input and be passed through a log lifecycle
 *
 * event is alive from zlog_init to zlog_fini, but content of event is
 * refreshed in every zlog,vlog or hzlog
 */

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include "zc_defs.h"

/**
 * control how the log to be writen, hexadecimal or by printf format
 */
typedef enum {
	ZLOG_FMT = 0,
	ZLOG_HEX = 1,
} zlog_event_cmd;

/**
 * zlog event struct
 */
typedef struct {
	char *category_name;		/**< point to a category.name */
	size_t *category_name_len;	/**< point to a category.name_len */
	char host_name[256 + 1];	/**< fill by gethostname */
	size_t host_name_len;		/**< strlen(hostname) */

	char *file;			/**< source file name */
	long line;			/**< source file line */
	int level;			/**< level of the log this time */

	void *hex_buf;			/**< if use hzlog(), point to user's hex_buf */
	size_t hex_buf_len;		
	char *str_format;		/**< if use zlog() or vzlog(), printf format */
	va_list str_args;		/**< if use zlog() or vzlog(), printf arguements */
	zlog_event_cmd generate_cmd;	/**< cmd */

	struct timeval time_stamp;	/**< will be assign when needed */
	struct tm local_time;		/**< local time of time_stamp */
	char us[6 + 1];			/**< microsecond */
	char time_fmt_msus[MAXLEN_CFG_LINE + 1];
					/**< a_spec->time_fmt + ms||us = time_fmt for thread&now */

	pid_t pid;			/**< process id, getpid() */
	pthread_t tid;			/**< thread id, pthread_self */
} zlog_event_t;

zlog_event_t *zlog_event_new(void);

void zlog_event_del(zlog_event_t * a_event);

void zlog_event_refresh(zlog_event_t * a_event,
			char *category_name, size_t * category_name_len,
			char *file, long line, int level,
			void *hex_buf, size_t hex_buf_len, char *str_format,
			va_list str_args, int generate_cmd);

#endif
