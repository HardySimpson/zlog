/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zc_defs_h
#define __zc_defs_h

#include "zc_profile.h"
#include "zc_arraylist.h"
#include "zc_hashtable.h"
#include "zc_xplatform.h"
#include "zc_util.h"

#ifdef _MSC_VER
#define snprintf(str, size, format, __VA_ARGS__) _snprintf_s(str, size, 10240, format, __VA_ARGS__)
#define int32 int
#endif
#endif
