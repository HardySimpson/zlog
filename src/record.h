/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_record_h
#define __zlog_record_h

#include "zc_defs.h"

typedef int (*zlog_record_fn)(char *msg, size_t mlen, char *path, size_t plen);

typedef struct zlog_record_s {
	zc_sds name;
	zlog_record_fn output;
} zlog_record_t;

zlog_record_t *zlog_record_new(const char *name, zlog_record_fn output);
void zlog_record_del(zlog_record_t *a_record);
void zlog_record_profile(zlog_record_t *a_record, int flag);

#endif
