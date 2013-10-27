/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_deepness_h
#define __zlog_deepness_h

typedef struct zlog_deepness_s zlog_deepness_t;

struct zlog_deepness_s {
	zc_sds sign;
	unsigned int perm;
	size_t buffer_len;
	size_t flush_len;
	size_t flush_count;
	ssize_t fsync_count;
};

zlog_deepness_t *zlog_deepness_new(char *line);
void zlog_deepness_del(zlog_deepness_t * a_deepness);
void zlog_deepness_profile(zlog_deepness_t * a_deepness, int flag);

#define zlog_deepness_has_name(a_deepness, fname) \
	STRCMP(a_deepness->name, ==, fname)

#endif
