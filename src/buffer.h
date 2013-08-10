/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#ifndef __zlog_buffer_h
#define __zlog_buffer_h

/* buffer, is a dynamic expand buffer for one single log,
 * as one single log will interlace if use multiple write() to file.
 * and buffer is always keep in a thread, to make each thread has its
 * own bufferfer to avoid lock.
 */

#include <stdarg.h>
#include <stdint.h>

typedef struct zlog_buffer_s {
	zc_sds buf;
	size_t size;
} zlog_buffer_t;


zlog_buffer_t *zlog_buf_new(size_t min, size_t max, const char *truncate_str);
void zlog_buffer_del(zlog_buf_t * a_buf);
void zlog_buffer_profile(zlog_buf_t * a_buf, int flag);

int zlog_buffer_vprintf(zlog_buf_t * a_buf, const char *format, va_list args);
int zlog_buffer_append(zlog_buf_t * a_buf, const char *str, size_t str_len);
int zlog_buffer_adjust_append(zlog_buf_t * a_buf, const char *str, size_t str_len,
			int left_adjust, size_t in_width, size_t out_width);
int zlog_buffer_printf_dec32(zlog_buf_t * a_buf, uint32_t ui32, int width);
int zlog_buffer_printf_dec64(zlog_buf_t * a_buf, uint64_t ui64, int width);
int zlog_buffer_printf_hex(zlog_buf_t * a_buf, uint32_t ui32, int width);

#define zlog_buffer_restart(a_buf) do { \
	a_buffer->tail = a_buf->start; \
} while(0)

#define zlog_buffer_len(a_buf) (a_buf->tail - a_buf->start)
#define zlog_buffer_str(a_buf) (a_buf->start)
#define zlog_buffer_seal(a_buf) do {*(a_buf)->tail = '\0';} while (0)

#endif
