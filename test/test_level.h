/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __test_level_h
#define __test_level_h

#include "zlog.h"

enum {
	ZLOG_LEVEL_TRACE = 30,
	/* must equals conf file setting */
};

#define zlog_trace(cat, format, args...) \
	zlog(cat, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_TRACE, format, ##args)

#endif
