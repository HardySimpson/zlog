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

#ifndef __zc_error_h
#define __zc_error_h

#include <stdarg.h>

#define zc_debug(fmt, args...) \
	zc_debug_inner(__FILE__, __LINE__, fmt, ## args)

#define zc_error(fmt, args...) \
	zc_error_inner(__FILE__, __LINE__, fmt, ## args)

#ifdef DEBUG
	/* for debug , in real world  turn off*/
	#define zc_assert_debug(expr,rv) \
		if(!(expr)) { \
			zc_error(#expr" is null"); \
			return rv; \
		} 
#else 
	#define zc_assert_debug(expr,rv)
#endif

/* for runtime */
#define zc_assert_runtime(expr,rv) \
	if(!(expr)) { \
		zc_error(#expr" is null"); \
		return rv; \
	} 

int zc_debug_inner(const char *file, const long line, const char *fmt,
			  ...);
int zc_error_inner(const char *file, const long line, const char *fmt,
			  ...);

#endif
