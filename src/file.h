/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_file_h
#define __zlog_file_h

/* file, is the zlog version of stdio's FILE with buffer IO
 * why zlog reinvent FILE?
 * for performance, a good-designed buffer will make the library run in the top speed of disk
 * for atomic write single log, as stdio can't control what time will call write
 * for multiple threads, each has its own file table without lock
 */

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

typedef struct zlog_file_s {
	int fd;
	zc_sds path; 
	zc_sds buf;
	size_t buf_cnt;
} zlog_file_t;

zlog_file_t *zlog_file_open(sds path, size_t buf_size);
void zlog_file_close(zlog_file_t * a_file);
void zlog_file_profile(zlog_file_t * a_file, int flag);

int zlog_file_vprintf(zlog_file_t * a_file, const char *format, va_list args);
int zlog_file_append(zlog_file_t * a_file, const char *str, size_t str_len);
int zlog_file_adjust_append(zlog_file_t * a_file, const char *str, size_t str_len,
			int left_adjust, size_t in_width, size_t out_width);
int zlog_file_printf_dec32(zlog_file_t * a_file, uint32_t ui32, int width);
int zlog_file_printf_dec64(zlog_file_t * a_file, uint64_t ui64, int width);
int zlog_file_printf_hex(zlog_file_t * a_file, uint32_t ui32, int width);

#endif
