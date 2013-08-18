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
			zc_sds a_buffer);

struct zlog_spec_s {
	zc_sds pattern;

	zc_sds time_fmt;
	zc_sds time_str;
	time_t time_str_sec_cache;

	zc_sds mdc_key;

	zc_sds print_fmt;
	int align_left;
	size_t max_width;
	size_t min_width;

	zlog_spec_gen_fn gen;
};

zlog_spec_t *zlog_spec_new(char *pattern_start, char **pattern_end);
void zlog_spec_del(zlog_spec_t * a_spec);
void zlog_spec_profile(zlog_spec_t * a_spec, int flag);

#define zlog_spec_gen(a_spec, a_event, a_mdc, a_buffer) \
	a_spec->gen(a_spec, a_event, a_mdc, a_buffer)

#endif
