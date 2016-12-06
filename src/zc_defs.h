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
#include "win_compat.h"
#define tidname(x) (x->tid.p)
#elif
#define tidname(x) (x->tid)
#endif

#endif
