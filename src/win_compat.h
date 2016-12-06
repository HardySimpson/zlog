#ifndef __zc_win_compat_h
#define __zc_win_compat_h

#include <time.h>
#include <stdio.h>

#define snprintf(str, size, format, ...) _snprintf_s(str, size, 10240, format, __VA_ARGS__)
#define lstat(a,b) stat(a,b)
#define va_copy(d,s) ((d) = (s))

int strcasecmp(char *a,char *b);
struct tm *localtime_r(time_t *clock,struct tm *res);
int gettimeofday(struct timeval *tp,struct timezone *tz);

#endif  // __zc_win_compat_h