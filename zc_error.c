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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "zc_error.h"
#include "zc_xplatform.h"

static void zc_time(char *time_str, size_t time_str_size)
{
	time_t tt;
	struct tm local_time;

	time(&tt);
	localtime_r(&tt, &local_time);
	strftime(time_str, time_str_size, "%m-%d %X", &local_time);

	return;
}

int zc_debug_inner(const char *file, const long line, const char *fmt, ...)
{
	va_list args;
	int r;
	char *debug_log;
	FILE *fp;
	char time_str[20 + 1];

	debug_log = getenv("XLOG_DEBUG_LOG");
	if (!debug_log)
		return 0;

	fp = fopen(debug_log, "a");
	if (!fp)
		return -1;

	zc_time(time_str, sizeof(time_str));

	r = fprintf(fp, "%s DEBUG (%d:%s:%ld) ", time_str, getpid(), file, line);
	va_start(args, fmt);
	r += vfprintf(fp, fmt, args);
	va_end(args);
	r += fprintf(fp, "\n");

	fclose(fp);

	return r;
}

int zc_error_inner(const char *file, const long line, const char *fmt, ...)
{
	va_list args;
	int r;
	char *error_log;
	FILE *fp;
	char time_str[20 + 1];

	error_log = getenv("XLOG_ERROR_LOG");
	if (!error_log)
		return 0;

	fp = fopen(error_log, "a");
	if (!fp)
		return -1;

	zc_time(time_str, sizeof(time_str));
	r = fprintf(fp, "%s ERROR (%d:%s:%ld) ", time_str, getpid(), file, line);
	va_start(args, fmt);
	r += vfprintf(fp, fmt, args);
	va_end(args);
	r += fprintf(fp, "\n");

	fclose(fp);

	return r;
}
