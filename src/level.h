/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_level_h
#define __zlog_level_h

typedef struct zlog_level_s {
	int number;
	zc_sds str_upper;
	zc_sds str_lower;
       	int syslog_level;
} zlog_level_t;

zlog_level_t *zlog_level_new(char *line);
void zlog_level_del(zlog_level_t *a_level);
void zlog_level_profile(zlog_level_t *a_level, int flag);

#endif
