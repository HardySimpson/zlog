/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

#include "zc_defs.h"
#include "buf.h"

/*******************************************************************************/
static void xlog_buf_debug(xlog_buf_t * a_buf);
static int xlog_buf_set_size(xlog_buf_t * a_buf, size_t bufsize_min, size_t bufsize_max);
static int xlog_buf_resize(xlog_buf_t * a_buf, size_t increment);
/*******************************************************************************/
static int xlog_buf_set_size(xlog_buf_t * a_buf, size_t bufsize_min, size_t bufsize_max)
{
	if (bufsize_max < bufsize_min) {
		zc_error("bufsize_max[%ld] < bufsize_min[%ld]", bufsize_max, bufsize_min);
		return -1;
	}

	if (bufsize_min == 0) {
		a_buf->size_min = 1024;
		zc_debug("bufsize_min not set, use 1024 instead");
	} else {
		a_buf->size_min = bufsize_min;
	}

	a_buf->size_max = bufsize_max;
	if (a_buf->size_max != 0 && a_buf->size_max < a_buf->size_min) {
		zc_error("a_buf->size_max[%ld] < a_buf->size_min[%ld]", a_buf->size_max, a_buf->size_min);
		return -1;
	}

	if (a_buf->size_max > a_buf->size_min) {
		a_buf->size_step = (a_buf->size_max - a_buf->size_min) / 10;
	} else {
		a_buf->size_step = 0;
	}

	return 0;
}

xlog_buf_t *xlog_buf_new(size_t bufsize_min, size_t bufsize_max, const char *truncate_str)
{
	int rc = 0;
	xlog_buf_t *a_buf;

	a_buf = calloc(1, sizeof(*a_buf));
	if (!a_buf) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	if (truncate_str) {
		if (strlen(truncate_str) > sizeof(a_buf->truncate_str) - 1) {
			zc_error("truncate_str[%s] overflow", truncate_str);
			rc = -1;
			goto xlog_buf_create_exit;
		} else {
			strcpy(a_buf->truncate_str, truncate_str);
		}
		a_buf->truncate_str_len = strlen(truncate_str);
	}

	rc = xlog_buf_set_size(a_buf, bufsize_min, bufsize_max);
	if (rc) {
		zc_error("xlog_buf_set_size fail");
		goto xlog_buf_create_exit;
	}

	a_buf->start = calloc(1, a_buf->size_min);
	if (!a_buf->start) {
		zc_error("calloc fail, errno[%d]", errno);
		rc = -1;
		goto xlog_buf_create_exit;
	}
	a_buf->end = a_buf->start;
	a_buf->size_real = a_buf->size_min;

      xlog_buf_create_exit:
	if (rc) {
		xlog_buf_del(a_buf);
		return NULL;
	} else {
		xlog_buf_debug(a_buf);
		return a_buf;
	}
}

/*******************************************************************************/
void xlog_buf_del(xlog_buf_t * a_buf)
{
	zc_assert(a_buf,);

	if (a_buf->start) {
		free(a_buf->start);
	}
	zc_debug("free a_buf at[%p]", a_buf);
	free(a_buf);
	return;
}

/*******************************************************************************/
void xlog_buf_clean(xlog_buf_t * a_buf)
{
	zc_assert(a_buf,);

	memset(a_buf->start, 0x00, a_buf->size_real);
	a_buf->end = a_buf->start;
	return;
}

/*******************************************************************************/
void xlog_buf_restart(xlog_buf_t * a_buf)
{
	zc_assert(a_buf,);

	a_buf->end = a_buf->start;
	return;
}

/*******************************************************************************/
int xlog_buf_printf(xlog_buf_t * a_buf, const char *format, ...)
{
	int rc;
	va_list args;

	zc_assert(a_buf, -1);
	zc_assert(format, -1);

	va_start(args, format);
	rc = xlog_buf_vprintf(a_buf, format, args);
	va_end(args);

	return rc;
}

/*******************************************************************************/
static void xlog_buf_truncate(xlog_buf_t * a_buf)
{
	char *p;
	size_t len;

	if ((a_buf->truncate_str)[0] == '\0')
		return;

	p = (a_buf->end - a_buf->truncate_str_len);
	if (p < a_buf->start)
		p = a_buf->start;

	len = a_buf->end - p;

	memcpy(p, a_buf->truncate_str, len);

	return;
}

int xlog_buf_vprintf(xlog_buf_t * a_buf, const char *format, va_list args)
{
	int rc = 0;
	va_list ap;
	size_t size_left;
	int nwrite;

	zc_assert(a_buf, -1);

	if (format == NULL) {
		return 0;
	}

	if (a_buf->size_real < 0) {
		zc_error("pre-use of xlog_buf_resize fail, so can't convert");
		return -1;
	}

	while (1) {
		va_copy(ap, args);
		size_left = a_buf->size_real - (a_buf->end - a_buf->start);
		nwrite = vsnprintf(a_buf->end, size_left, format, ap);
		if (nwrite < 0) {
			zc_error("vsnprintf fail, errno[%d]", errno);
			zc_error("nwrite[%d], size_left[%ld], format[%s]", nwrite, size_left, format);
			zc_error(format, ap);
			return -1;
		} else if (nwrite >= size_left) {
			zc_debug("nwrite[%d]>=size_left[%ld],format[%s],resize", nwrite, size_left, format);
			rc = xlog_buf_resize(a_buf, 0);
			if (rc > 0) {
				zc_error("conf limit to %ld, can't extend, so truncate", a_buf->size_max);
				a_buf->end += size_left - 1;
				*(a_buf->end) = '\0';
				xlog_buf_truncate(a_buf);
				return 1;
			} else if (rc < 0) {
				zc_error("xlog_buf_resize fail");
				return -1;
			} else {
				zc_debug("xlog_buf_resize succ, to[%ld]", a_buf->size_real);
				continue;
			}
		} else {
			a_buf->end += nwrite;
			return 0;
		}
	}

	return 0;
}

/*******************************************************************************/
int xlog_buf_append(xlog_buf_t * a_buf, const char *str, size_t str_len)
{
	int rc = 0;
	size_t size_left;

	zc_assert(a_buf, -1);

	if (str == NULL || str_len <= 0) {
		return 0;
	}

	if (a_buf->size_real < 0) {
		zc_error("pre-use of xlog_buf_resize fail, so can't convert");
		return -1;
	}

	size_left = a_buf->size_real - (a_buf->end - a_buf->start);
	if (str_len > size_left - 1) {
		zc_debug("size_left not enough, resize");
		rc = xlog_buf_resize(a_buf, str_len - size_left + 1);
		if (rc > 0) {
			zc_error("conf limit to %ld, can't extend, so output", a_buf->size_max);
			memcpy(a_buf->end, str, size_left - 1);
			a_buf->end += size_left - 1;
			*(a_buf->end) = '\0';
			xlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			zc_error("xlog_buf_resize fail");
			return -1;
		} else {
			zc_debug("xlog_buf_resize succ, to[%ld]", a_buf->size_real);
		}
	}

	memcpy(a_buf->end, str, str_len);
	a_buf->end += str_len;
	*(a_buf->end) = '\0';
	return 0;
}

/*******************************************************************************/
int xlog_buf_strftime(xlog_buf_t * a_buf, const char *time_fmt, size_t time_len, const struct tm *a_tm)
{
	int rc = 0;
	size_t size_left;
	size_t nwrite;

	zc_assert(a_buf, -1);
	zc_assert(time_fmt, -1);
	zc_assert(a_tm, -1);

	if (time_len <= 0) {
		return 0;
	}

	if (a_buf->size_real < 0) {
		zc_error("pre-use of xlog_buf_resize fail, so can't convert");
		return -1;
	}

	size_left = a_buf->size_real - (a_buf->end - a_buf->start);
	if (time_len > size_left - 1) {
		zc_debug("size_left not enough, resize");
		rc = xlog_buf_resize(a_buf, time_len - size_left + 1);
		if (rc > 0) {
			zc_error("conf limit to %ld, can't extend, so trucate", a_buf->size_max);
			strftime(a_buf->end, size_left - 1, time_fmt, a_tm);
			a_buf->end += size_left - 1;
			a_buf->end = '\0';
			xlog_buf_truncate(a_buf);
			return 1;
		} else if (rc < 0) {
			zc_error("xlog_buf_resize fail");
			return -1;
		} else {
			zc_debug("xlog_buf_resize succ, to[%ld]", a_buf->size_real);
		}
	}

	size_left = a_buf->size_real - (a_buf->end - a_buf->start);
	nwrite = strftime(a_buf->end, size_left - 1, time_fmt, a_tm);
	a_buf->end += nwrite;
	*(a_buf->end) = '\0';

	if (nwrite <= 0) {
		zc_error("strftime maybe failed or output 0 char, nwrite[%d], time_fmt[%s]", nwrite, time_fmt);
	}

	return 0;
}

/*******************************************************************************/
/* return 0:	success
 * return <0:	fail, set size_real to -1;
 * return >0:	by conf limit, can't extend size
 */
static int xlog_buf_resize(xlog_buf_t * a_buf, size_t increment)
{
	int rc = 0;
	size_t new_size = 0;
	size_t len = 0;
	char *p = NULL;

	if (a_buf->size_max != 0 && a_buf->size_real >= a_buf->size_max) {
		zc_error("a_buf->size_real[%ld] >= a_buf->size_max[%ld]", a_buf->size_real, a_buf->size_max);
		return 1;
	}

	if (a_buf->size_max == 0 && increment == 0) {
		/* unlimit && use inner step */
		new_size = a_buf->size_real * 1.5;
	} else if (a_buf->size_max == 0 && increment != 0) {
		/* unlimit && use outer step */
		new_size = a_buf->size_real + 1.5 * increment;
	} else if (a_buf->size_max != 0 && increment == 0) {
		/* limit && use inner step */
		if (a_buf->size_real + a_buf->size_step < a_buf->size_max) {
			new_size = a_buf->size_real + a_buf->size_step;
		} else if (a_buf->size_real + a_buf->size_step >= a_buf->size_max) {
			new_size = a_buf->size_max;
			rc = 1;
		}
	} else if (a_buf->size_max != 0 && increment != 0) {
		/* limit && use out step */
		if (a_buf->size_real + increment < a_buf->size_max) {
			new_size = a_buf->size_real + a_buf->size_step;
		} else if (a_buf->size_real + increment >= a_buf->size_max) {
			new_size = a_buf->size_max;
			rc = 1;
		}
	}

	len = a_buf->end - a_buf->start;

	p = realloc(a_buf->start, new_size);
	if (!p) {
		zc_error("realloc fail, errno[%d]", errno);
		free(a_buf->start);
		a_buf->start = NULL;
		a_buf->end = NULL;
		/* set size_real = -1, so other func know buf is unavailiable */
		a_buf->size_real = -1;
		return -1;
	} else {
		a_buf->start = p;
		a_buf->end = p + len;
		memset(a_buf->end, 0x00, new_size - len);
		a_buf->size_real = new_size;
	}

	return rc;
}

/*******************************************************************************/
static void xlog_buf_debug(xlog_buf_t * a_buf)
{
	zc_debug("buf:[%p][%ld-%ld][%s][%p][%ld]", a_buf,
		a_buf->size_min, a_buf->size_max, a_buf->truncate_str,
		a_buf->start, a_buf->end - a_buf->start);
	return;
}

void xlog_buf_profile(xlog_buf_t * a_buf)
{
	zc_assert(a_buf, );

	zc_error("buf:[%p][%ld-%ld][%s][%p][%ld]", a_buf,
		a_buf->size_min, a_buf->size_max, a_buf->truncate_str,
		a_buf->start, a_buf->end - a_buf->start);
	return;
}
