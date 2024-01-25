/* Copyright (c) Hardy Simpson
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

	time(&tt);
#ifdef _WIN32
	struct tm *local_time;
	local_time = localtime(&tt);
	strftime(time_str, time_str_size, "%m-%d %H:%M:%S", local_time);
#else
	struct tm local_time;
	localtime_r(&tt, &local_time);
	strftime(time_str, time_str_size, "%m-%d %H:%M:%S", &local_time);
#endif

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

