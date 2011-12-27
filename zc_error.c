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
