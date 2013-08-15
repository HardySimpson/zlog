/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_spec_h
#define __zlog_spec_h

typedef struct zlog_spec_s zlog_spec_t;

typedef int (*zlog_spec_gen_fn) (zlog_spec_t * a_spec,
			zlog_event_t * a_event, zlog_mdc_t *a_mdc,
			zc_sds buffer);

struct zlog_spec_s {
	char *str;
	int len;

	zc_sds time_fmt;

	char time_fmt[MAXLEN_CFG_LINE + 1];
	int time_cache_index;
	char mdc_key[MAXLEN_PATH + 1];

	char print_fmt[MAXLEN_CFG_LINE + 1];
	int left_adjust;
	size_t max_width;
	size_t min_width;

	zlog_spec_gen_fn gen;
};

zlog_spec_t *zlog_spec_new(char *pattern_start, char **pattern_end, int * time_cache_count);
void zlog_spec_del(zlog_spec_t * a_spec);
void zlog_spec_profile(zlog_spec_t * a_spec, int flag);

#define zlog_spec_gen(a_spec, event, mdc, buffer) \
	a_spec->gen(a_spec, event, mdc, buffer)

#endif
