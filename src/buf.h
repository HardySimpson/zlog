/* Copyright (c) Hardy Simpson
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __zlog_buf_h
#define __zlog_buf_h

/* buf, is a dynamic expand buffer for one single log,
 * as one single log will interlace if use multiple write() to file.
 * and buf is always keep in a thread, to make each thread has its
 * own buffer to avoid lock.
 */

#include <stdarg.h>
#include <stdint.h>

/* include for MAXLEN_PATH */
#include "zc_xplatform.h"

typedef struct zlog_buf_s {
	char *start;
	char *tail;
	char *end;
	char *end_plus_1;

	size_t size_min;
	size_t size_max;
	size_t size_real;

	char truncate_str[MAXLEN_PATH + 1];
	size_t truncate_str_len;
} zlog_buf_t;


zlog_buf_t *zlog_buf_new(size_t min, size_t max, const char *truncate_str);
void zlog_buf_del(zlog_buf_t * a_buf);
void zlog_buf_profile(zlog_buf_t * a_buf, int flag);

int zlog_buf_vprintf(zlog_buf_t * a_buf, const char *format, va_list args);
int zlog_buf_append(zlog_buf_t * a_buf, const char *str, size_t str_len);
int zlog_buf_adjust_append(zlog_buf_t * a_buf, const char *str, size_t str_len,
			int left_adjust, int zero_pad, size_t in_width, size_t out_width);
int zlog_buf_printf_dec32(zlog_buf_t * a_buf, uint32_t ui32, int width);
int zlog_buf_printf_dec64(zlog_buf_t * a_buf, uint64_t ui64, int width);
int zlog_buf_printf_hex(zlog_buf_t * a_buf, uint32_t ui32, int width);

#define zlog_buf_restart(a_buf) do { \
	a_buf->tail = a_buf->start; \
} while(0)

#define zlog_buf_len(a_buf) (a_buf->tail - a_buf->start)
#define zlog_buf_str(a_buf) (a_buf->start)
#define zlog_buf_seal(a_buf) do {*(a_buf)->tail = '\0';} while (0)

#endif
