/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_fmacro_h
#define __zlog_fmacro_h

#define _BSD_SOURCE

#if defined(__linux__) || defined(__OpenBSD__) || defined(_AIX)
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#else
#define _XOPEN_SOURCE
#endif

#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#endif
