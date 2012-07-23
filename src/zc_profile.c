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

#include "fmacros.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "zc_profile.h"
#include "zc_xplatform.h"

static void zc_time(char *time_str, size_t time_str_size)
{
	time_t tt;
	struct tm local_time;

	time(&tt);
	localtime_r(&tt, &local_time);
	strftime(time_str, time_str_size, "%m-%d %T", &local_time);

	return;
}

int zc_profile_inner(int flag, const char *file, const long line, const char *fmt, ...)
{
	va_list args;
	char time_str[20 + 1];
	FILE *fp = NULL;

	static char *debug_log = NULL;
	static char *error_log = NULL;
	static size_t init_flag = 0;

	if (!init_flag) {
		init_flag = 1;
		debug_log = getenv("ZLOG_PROFILE_DEBUG");
		error_log = getenv("ZLOG_PROFILE_ERROR");
	}

	switch (flag) {
	case ZC_DEBUG:
 		if (debug_log == NULL) return 0;
		fp = fopen(debug_log, "a");
		if (!fp) return -1;
		zc_time(time_str, sizeof(time_str));
		fprintf(fp, "%s DEBUG (%d:%s:%ld) ", time_str, getpid(), file, line);
		break;
	case ZC_WARN:
 		if (error_log == NULL) return 0;
		fp = fopen(error_log, "a");
		if (!fp) return -1;
		zc_time(time_str, sizeof(time_str));
		fprintf(fp, "%s WARN  (%d:%s:%ld) ", time_str, getpid(), file, line);
		break;
	case ZC_ERROR:
 		if (error_log == NULL) return 0;
		fp = fopen(error_log, "a");
		if (!fp) return -1;
		zc_time(time_str, sizeof(time_str));
		fprintf(fp, "%s ERROR (%d:%s:%ld) ", time_str, getpid(), file, line);
		break;
	}

	/* writing file twice(time & msg) is not atomic
	 * may cause cross
	 * but avoid log size limit */
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fprintf(fp, "\n");

	fclose(fp);
	return 0;
}

