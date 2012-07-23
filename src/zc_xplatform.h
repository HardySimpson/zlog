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
#ifndef __zc_xplatform_h
#define __zc_xplatform_h

#include <limits.h>

#define ZLOG_INT32_LEN   sizeof("-2147483648") - 1
#define ZLOG_INT64_LEN   sizeof("-9223372036854775808") - 1

#if ((__GNU__ == 2) && (__GNUC_MINOR__ < 8))
#define ZLOG_MAX_UINT32_VALUE  (uint32_t) 0xffffffffLL
#else
#define ZLOG_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#endif

#define ZLOG_MAX_INT32_VALUE   (uint32_t) 0x7fffffff

#define MAXLEN_PATH 1024
#define MAXLEN_CFG_LINE (MAXLEN_PATH * 4)

#define FILE_NEWLINE "\n"
#define FILE_NEWLINE_LEN 1

#include <string.h>
#include <strings.h>

#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#define STRICMP(_a_,_C_,_b_) ( strcasecmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strncasecmp(_a_,_b_,_n_) _C_ 0 )

#endif
