#ifndef __zc_error_h
#define __zc_error_h

#include <stdarg.h>

#define zc_debug(fmt, args...) \
	zc_debug_inner(__FILE__, __LINE__, fmt, ## args)

#define zc_error(fmt, args...) \
	zc_error_inner(__FILE__, __LINE__, fmt, ## args)

#define zc_assert(expr,rv) \
	if(!(expr)) { \
		zc_error("expr is null"); \
		return rv; \
	}

extern int zc_debug_inner(const char *file, const long line, const char *fmt, ...);
extern int zc_error_inner(const char *file, const long line, const char *fmt, ...);

#endif
