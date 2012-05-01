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

#define zc_assert(expr,rv) \
	if(!(expr)) { \
		zc_error(#expr" is null or 0"); \
		return rv; \
	} 

enum zc_profile_flag {
	ZC_DEBUG = 0,
	ZC_ERROR = 1
};

#define zc_debug(fmt, args...) \
	zc_profile_inner(ZC_DEBUG, __FILE__, __LINE__, fmt, ## args)

#define zc_error(fmt, args...) \
	zc_profile_inner(ZC_ERROR, __FILE__, __LINE__, fmt, ## args)

#define zc_profile(flag, fmt, args...) \
	zc_profile_inner(flag, __FILE__, __LINE__, fmt, ## args)

int zc_profile_inner(int flag, 
		const char *file, const long line,
		const char *fmt, ...);

void zc_profile_init(void);

#endif
