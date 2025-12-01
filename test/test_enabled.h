/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2018 by Teracom Telemática S/A
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

#ifndef __test_level_h
#define __test_level_h

#include "zlog.h"

enum {
	ZLOG_LEVEL_TRACE = 30,
	/* must equals conf file setting */
};

#define zlog_trace(cat, ...)                                                                       \
    zlog(cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__,            \
         ZLOG_LEVEL_TRACE, __VA_ARGS__)

#define zlog_trace_enabled(cat) zlog_level_enabled(cat, ZLOG_LEVEL_TRACE)

#endif
