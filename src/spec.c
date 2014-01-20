/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 */

#include "fmacros.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "conf.h"
#include "spec.h"
#include "level_list.h"
#include "zc_defs.h"


#define ZLOG_DEFAULT_TIME_FMT "%F %T"
#define	ZLOG_HEX_HEAD  \
	"\n             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF"

/*******************************************************************************/
void zlog_spec_profile(zlog_spec_t * a_spec, int flag)
{
	zc_assert(a_spec,);
	zc_profile(flag, "----spec[%p][%.*s][%s|%d][%s,%ld,%ld][%s]----",
		a_spec,
		a_spec->len, a_spec->str,
		a_spec->time_fmt,
		a_spec->time_cache_index,
		a_spec->print_fmt, (long)a_spec->max_width, (long)a_spec->min_width,
		a_spec->mdc_key);
	return;
}

/*******************************************************************************/
/* implementation of write function */

static int zlog_spec_gen_time(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	time_t now_sec = a_event->time_now.tv_sec;
	struct tm *time_local = &(a_event->time_local);
	zc_sds rs;
	int rc;

	/* the event meet the 1st time_spec in his life cycle */
	if (!now_sec) {
		rc = gettimeofday(&(a_event->time_now), NULL);
		if (!rc) { zc_error("gettimeofday fail, errno[%d]", errno); return -1;}
		now_sec = a_event->time_now.now;
	}

	/* When this event's last cached time_local is not now */
	if (a_event->time_local_sec_cache != now_sec) {
		time_local = localtime_r(&(now_sec), &(a_event->time_local));
		if (!time_local) { zc_error("localtime_r fail, errno[%d]", errno); return -1;}
		a_event->time_local_sec_cache = now_sec;
	}

	/* When this spec's last cache time string is not now */
	if (a_spec->time_str_sec_cache != now_sec) {
		rs = zc_sdsstrftime(a_spec->time_str, a_spec->time_fmt, time_local);
		if (!rs) { zc_error("zc_sdsstrftime fail"); return -1; }
		a_spec->time_str_sec_cache = now_sec;
	}

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_spec->time_str,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_spec->time_str);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_msec(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	int rc;
	zc_sds rs;
	char msec[3];
	int ms;

	if (!a_event->time_now.tv_sec) {
		rc = gettimeofday(&(a_event->time_now), NULL);
		if (!rc) { zc_error("gettimeofday fail, errno[%d]", errno); return -1;}
	}

	ms = a_event->time_now.tv_usec / 1000;
	msec[2] = ms % 10 + '0'; ms /= 10;
	msec[1] = ms % 10 + '0'; ms /= 10;
	msec[0] = ms % 10 + '0';

	if (a_spec->print_fmt) {
		rs = zc_sdscatlen_adjust(a_buffer, msec, 3
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatlen(a_buffer, msec, 3);
		if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_usec(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	int rc;
	zc_sds rs;
	char usec[6];
	int us;

	if (!a_event->time_now.tv_sec) {
		rc = gettimeofday(&(a_event->time_now), NULL);
		if (!rc) { zc_error("gettimeofday fail, errno[%d]", errno); return -1;}
	}

	us = a_event->time_now.tv_usec
	usec[5] = us % 10 + '0'; us /= 10;
	usec[4] = us % 10 + '0'; us /= 10;
	usec[3] = us % 10 + '0'; us /= 10;
	usec[2] = us % 10 + '0'; us /= 10;
	usec[1] = us % 10 + '0'; us /= 10;
	usec[0] = us % 10 + '0';

	if (a_spec->print_fmt) {
		rs = zc_sdscatlen_adjust(a_buffer, usec, 6
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatlen(a_buffer, msec, 3);
		if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_mdc(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;
	zlog_mdc_kv_t *kv;

	kv = zlog_mdc_get_kv(mdc, a_spec->mdc_key);
	if (!kv) { zc_error("zlog_mdc_get_kv key[%s] fail", a_spec->mdc_key); return 0; }

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, kv->value,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, kv->value);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_str(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_spec->pattern,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_spec->pattern);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_category(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_event->category_name,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_event->category_name);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_srcfile(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_spec->print_fmt) {
		rs = zc_sdscatlen_adjust(a_buffer, a_event->srcfile, a_event->srcfile_len,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatlen(a_buffer, a_event->srcfile, a_event->srcfile_len);
		if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_srcfile_neat(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;
	char *end;
	char *p;
	size_t len;

	end = a_event->srcfile + a_event->srcfile_len;
	for (p = end; p > a_event->srcfile; p--) {
		if (*p == '/') {
			len = end - p;
			if (a_spec->print_fmt) {
				rs = zc_sdscatlen_adjust(a_buffer, p, len,
					a_spec->align_left, a_spec->max_width, a_spec->min_width);
				if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
			} else {
				rs = zc_sdscatlen(a_buffer, p, len);
				if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
			}
			return 0;
		}
	}

	/* else back to normal */
	return zlog_spec_gen_srcfile(a_spec, a_event, a_mdc, a_buffer);
}

static int zlog_spec_gen_srcline(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;
	unsigned char *p;
	unsigned char tmp[ZLOG_INT64_LEN + 1];
	size_t len;
	uint32_t ui32;
	uint64_t ui64;

	p = tmp + ZLOG_INT64_LEN;
	if (a_event->line <= ZLOG_MAX_UINT32_VALUE) {
		/*
		* To divide 64-bit numbers and to find remainders
		* on the x86 platform gcc and icc call the libc functions
		* [u]divdi3() and [u]moddi3(), they call another function
		* in its turn.  On FreeBSD it is the qdivrem() function,
		* its source code is about 170 lines of the code.
		* The glibc counterpart is about 150 lines of the code.
		*
		* For 32-bit numbers and some divisors gcc and icc use
		* a inlined multiplication and shifts.  For example,
		* unsigned "i32 / 10" is compiled to
		*
		*     (i32 * 0xCCCCCCCD) >> 35
		*/

		ui32 = (uint32_t) ui64;

		do {
			*--p = (unsigned char) (ui32 % 10 + '0');
		} while (ui32 /= 10);

	} else {
		ui64 = a_event->line;
		do {
			*--p = (unsigned char) (ui64 % 10 + '0');
		} while (ui64 /= 10);
	}


	/* zero or space padding */
	len = (tmp + ZLOG_INT64_LEN) - p;

	if (a_spec->print_fmt) {
		rs = zc_sdscatlen_adjust(a_buffer, p, len,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatlen(a_buffer, p, len);
		if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_srcfunc(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_spec->print_fmt) {
		rs = zc_sdscatlen_adjust(a_buffer, a_event->srcfunc, a_event->srcfunc_len,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatlen(a_buffer, a_event->srcfunc, a_event->srcfunc_len);
		if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
	}

	return 0;
}


static int zlog_spec_gen_hostname(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_event->hostname,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_event->hostname);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_newline(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_spec->print_fmt) {
		rs = zc_sdscatlen_adjust(a_buffer, FILE_NEWLINE, FILE_NEWLINE_LEN,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatlen(a_buffer, FILE_NEWLINE, FILE_NEWLINE_LEN);
		if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_percent(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_spec->print_fmt) {
		rs = zc_sdscatlen_adjust(a_buffer, "%", 1,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatlen_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatlen(a_buffer, "%", 1);
		if (!rs) { zc_error("zc_sdscatlen fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_pid(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;
	/* 1st in event lifecycle */
	if (!a_event->pid) {
		a_event->pid = getpid();

		/* compare with previous event */
		if (a_event->pid != a_event->last_pid) {
			a_event->last_pid = a_event->pid;
			zc_sdsclear(a_event->pid_str);
			rs = zc_sdscatprintf(a_event->pid_str, "%u", a_thread->event->pid);
			if (!rs) { zc_error("zc_sdscatprintf fail, errno[%d]", errno); return -1; }
		}
	}

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_event->pid_str,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_event->pid_str);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_tid_hex(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;
	/* don't need to get tid again, as tmap_new_thread fetch it already */
	/* and fork not change tid */
	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_event->tid_hex_str,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_event->tid_hex_str);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_tid_long(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;
	/* don't need to get tid again, as tmap_new_thread fetch it already */
	/* and fork not change tid */
	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_event->tid_str,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_event->tid_str);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_level_uppercase(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zlog_level_t *a_level;
	zc_sds rs;

	a_level = zlog_level_list_get(a_spec->levels, a_event->level);

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_level->str_upper,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_level->str_upper);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_level_lowercase(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zlog_level_t *a_level;
	zc_sds rs;

	a_level = zlog_level_list_get(a_spec->levels, a_event->level);

	if (a_spec->print_fmt) {
		rs = zc_sdscatsds_adjust(a_buffer, a_level->str_lower,
			a_spec->align_left, a_spec->max_width, a_spec->min_width);
		if (!rs) { zc_error("zc_sdscatsds_adjust fail, errno[%d]", errno); return -1; }
	} else {
		rs = zc_sdscatsds(a_buffer, a_level->str_lower);
		if (!rs) { zc_error("zc_sdscatsds fail, errno[%d]", errno); return -1; }
	}

	return 0;
}

static int zlog_spec_gen_usrmsg(zlog_spec_t * a_spec, zlog_event_t * a_event, zlog_mdc_t *a_mdc, zc_sds a_buffer)
{
	zc_sds rs;

	if (a_event->generate_cmd == ZLOG_FMT) {
		if (a_spec->print_fmt) {
			rs = zc_sdscatvprintf_adjust(a_buffer,
				a_event->str_format, a_event->str_args,
				a_spec->align_left, a_spec->max_width, a_spec->min_width);
			if (!rs) { zc_error("zc_sdscatvprintf_adjust fail, errno[%d]", errno); return -1; }
		} else {
			rs = zc_sdscatvprintf(a_buffer, a_event->str_format, a_event->str_args);
			if (!rs) { zc_error("zc_sdscatvprintf fail, errno[%d]", errno); return -1; }
		}
	} else if (a_event->generate_cmd == ZLOG_HEX) {
		if (a_spec->print_fmt) {
			rs = zc_sdscathex_adjust(a_buffer,
				a_event->hex_buf, a_event->hex_buf_len,
				a_spec->align_left, a_spec->max_width, a_spec->min_width);
			if (!rs) { zc_error("zc_sdscatvprintf_adjust fail, errno[%d]", errno); return -1; }
		} else {
			rs = zc_sdscatvprintf(a_buffer, a_event->hex_buf, a_event->hex_buf_len);
			if (!rs) { zc_error("zc_sdscatvprintf fail, errno[%d]", errno); return -1; }
		}
	}

	return 0;
}

#if 0
static int zlog_spec_write_usrmsg(zlog_spec_t * a_spec, zlog_thread_t * a_thread, zlog_buf_t * a_buf)
{
	if (a_thread->event->generate_cmd == ZLOG_FMT) {
		if (a_thread->event->str_format) {
			return zlog_buf_vprintf(a_buf,
				      a_thread->event->str_format,
				      a_thread->event->str_args);
		} else {
			return zlog_buf_append(a_buf, "format=(null)", sizeof("format=(null)")-1);
		}
	} else if (a_thread->event->generate_cmd == ZLOG_HEX) {
		int rc;
		long line_offset;
		long byte_offset;

		/* thread buf start == null or len <= 0 */
		if (a_thread->event->hex_buf == NULL) {
			rc = zlog_buf_append(a_buf, "buf=(null)", sizeof("buf=(null)")-1);
			goto zlog_hex_exit;
		}

		rc = zlog_buf_append(a_buf, ZLOG_HEX_HEAD, sizeof(ZLOG_HEX_HEAD)-1);
		if (rc) {
			goto zlog_hex_exit;
		}

		line_offset = 0;
		byte_offset = 0;

		while (1) {
			unsigned char c;

			rc = zlog_buf_append(a_buf, "\n", 1);
			if (rc)  goto zlog_hex_exit;

			rc = zlog_buf_printf_dec64(a_buf, line_offset + 1, 10);
			if (rc)  goto zlog_hex_exit;
			rc = zlog_buf_append(a_buf, "   ", 3);
			if (rc)  goto zlog_hex_exit;

			for (byte_offset = 0; byte_offset < 16; byte_offset++) {
				if (line_offset * 16 + byte_offset < a_thread->event->hex_buf_len) {
					c = *((unsigned char *)a_thread->event->hex_buf
						+ line_offset * 16 + byte_offset);
					rc = zlog_buf_printf_hex(a_buf, c, 2);
					if (rc) goto zlog_hex_exit;
					rc = zlog_buf_append(a_buf, " ", 1);
					if (rc) goto zlog_hex_exit;
				} else {
					rc = zlog_buf_append(a_buf, "   ", 3);
					if (rc)  goto zlog_hex_exit;
				}
			}

			rc = zlog_buf_append(a_buf, "  ", 2);
			if (rc) goto zlog_hex_exit;

			for (byte_offset = 0; byte_offset < 16; byte_offset++) {
				if (line_offset * 16 + byte_offset < a_thread->event->hex_buf_len) {
					c = *((unsigned char *)a_thread->event->hex_buf
						+ line_offset * 16 + byte_offset);
					if (c >= 32 && c <= 126) {
						rc = zlog_buf_append(a_buf,(char*)&c, 1);
						if (rc)  goto zlog_hex_exit;
					} else {
						rc = zlog_buf_append(a_buf, ".", 1);
						if (rc)  goto zlog_hex_exit;
					}
				} else {
					rc = zlog_buf_append(a_buf, " ", 1);
					if (rc)  goto zlog_hex_exit;
				}
			}

			if (line_offset * 16 + byte_offset >= a_thread->event->hex_buf_len) {
				break;
			}

			line_offset++;
		}

	      zlog_hex_exit:
		if (rc < 0) {
			zc_error("write hex msg fail");
			return -1;
		} else if (rc > 0) {
			zc_error("write hex msg, buf is full");
			return 1;
		}

		return 0;
	}

	return 0;
}
#endif

/*******************************************************************************/
static int zlog_spec_parse_print_fmt(zlog_spec_t * a_spec)
{
	/* -12.35 12 .35 */
	char *p, *q;
	long i, j;

	p = a_spec->print_fmt;
	if (*p == '-') {
		a_spec->left_adjust = 1;
		p++; 
	} else {
		a_spec->left_adjust = 0;
	}

	i = j = 0;
	sscanf(p, "%ld.", &i);
	q = strchr(p, '.');
	if (q) sscanf(q, ".%ld", &j);

	a_spec->min_width = (size_t) i;
	a_spec->max_width = (size_t) j;
	return 0;
}

/*******************************************************************************/
void zlog_spec_del(zlog_spec_t * a_spec)
{
	zc_assert(a_spec,);
	free(a_spec);
	zc_debug("zlog_spec_del[%p]", a_spec);
}

/* a spec may consist of
 * a const string: /home/bb
 * a string begin with %: %12.35d(%F %X,%l)
 */
zlog_spec_t *zlog_spec_new(char *pattern_start, char **pattern_next, zc_arraylist_t *levels)
{
	zlog_spec_t *a_spec;

	zc_assert(pattern_start, NULL);
	zc_assert(pattern_next, NULL);

	a_spec = calloc(1, sizeof(zlog_spec_t));
	if (!a_spec) { zc_error("calloc fail, errno[%d]", errno); return NULL; }

	a_spec->str = p = pattern_start;

	switch (*p) {
	case '%':
		/* a string begin with %: %12.35d(%F %X) */

		/* process width and precision char in %-12.35P */
		nread = 0;
		nscan = sscanf(p, "%%%[.0-9-]%n", a_spec->print_fmt, &nread);
		if (nscan == 1) {
			a_spec->gen_msg = zlog_spec_gen_msg_reformat;
			a_spec->gen_path = zlog_spec_gen_path_reformat;
			a_spec->gen_archive_path = zlog_spec_gen_archive_path_reformat;
			if (zlog_spec_parse_print_fmt(a_spec)) {
				zc_error("zlog_spec_parse_print_fmt fail");
				goto err;
			}
		} else {
			nread = 1; /* skip the % char */
			a_spec->gen_msg = zlog_spec_gen_msg_direct;
			a_spec->gen_path = zlog_spec_gen_path_direct;
			a_spec->gen_archive_path = zlog_spec_gen_archive_path_direct;
		}

		p += nread;

		if (*p == 'd') {
			if (*(p+1) != '(') {
				/* without '(' , use default */
				strcpy(a_spec->time_fmt, ZLOG_DEFAULT_TIME_FMT);
				p++;
			} else if (STRNCMP(p, ==, "d()", 3)) {
				/* with () but without detail time format,
				 * keep a_spec->time_fmt=="" */
				strcpy(a_spec->time_fmt, ZLOG_DEFAULT_TIME_FMT);
				p += 3;
			} else {
				nread = 0;
				nscan = sscanf(p, "d(%[^)])%n", a_spec->time_fmt, &nread);
				if (nscan != 1) {
					nread = 0;
				}
				p += nread;
				if (*(p - 1) != ')') {
					zc_error("in string[%s] can't find match \')\'", a_spec->str);
					goto err;
				}
			}

			a_spec->time_cache_index = *time_cache_count;
			(*time_cache_count)++;
			a_spec->write_buf = zlog_spec_write_time;

			*pattern_next = p;
			a_spec->len = p - a_spec->str;
			break;
		}

		if (*p == 'M') {
			nread = 0;
			nscan = sscanf(p, "M(%[^)])%n", a_spec->mdc_key, &nread);
			if (nscan != 1) {
				nread = 0;
				if (STRNCMP(p, ==, "M()", 3)) {
					nread = 3;
				}
			}
			p += nread;
			if (*(p - 1) != ')') {
				zc_error("in string[%s] can't find match \')\'", a_spec->str);
				goto err;
			}

			*pattern_next = p;
			a_spec->len = p - a_spec->str;
			a_spec->write_buf = zlog_spec_write_mdc;
			break;
		}

		if (STRNCMP(p, ==, "ms", 2)) {
			p += 2;
			*pattern_next = p;
			a_spec->len = p - a_spec->str;
			a_spec->write_buf = zlog_spec_write_ms;
			break;
		} else if (STRNCMP(p, ==, "us", 2)) {
			p += 2;
			*pattern_next = p;
			a_spec->len = p - a_spec->str;
			a_spec->write_buf = zlog_spec_write_us;
			break;
		}

		*pattern_next = p + 1;
		a_spec->len = p - a_spec->str + 1;

		switch (*p) {
		case 'c':
			a_spec->write_buf = zlog_spec_write_category;
			break;
		case 'D':
			strcpy(a_spec->time_fmt, ZLOG_DEFAULT_TIME_FMT);
			a_spec->time_cache_index = *time_cache_count;
			(*time_cache_count)++;
			a_spec->write_buf = zlog_spec_write_time;
			break;
		case 'F':
			a_spec->write_buf = zlog_spec_write_srcfile;
			break;
		case 'f':
			a_spec->write_buf = zlog_spec_write_srcfile_neat;
			break;
		case 'H':
			a_spec->write_buf = zlog_spec_write_hostname;
			break;
		case 'L':
			a_spec->write_buf = zlog_spec_write_srcline;
			break;
		case 'm':
			a_spec->write_buf = zlog_spec_write_usrmsg;
			break;
		case 'n':
			a_spec->write_buf = zlog_spec_write_newline;
			break;
		case 'p':
			a_spec->write_buf = zlog_spec_write_pid;
			break;
		case 'U':
			a_spec->write_buf = zlog_spec_write_srcfunc;
			break;
		case 'v':
			a_spec->write_buf = zlog_spec_write_level_lowercase;
			break;
		case 'V':
			a_spec->write_buf = zlog_spec_write_level_uppercase;
			break;
		case 't':
			a_spec->write_buf = zlog_spec_write_tid_hex;
			break;
		case 'T':
			a_spec->write_buf = zlog_spec_write_tid_long;
			break;
		case '%':
			a_spec->write_buf = zlog_spec_write_percent;
			break;
		default:
			zc_error("str[%s] in wrong format, p[%c]", a_spec->str, *p);
			goto err;
		}
		break;
	default:
		/* a const string: /home/bb */
		*pattern_next = strchr(p, '%');
		if (*pattern_next) {
			a_spec->len = *pattern_next - p;
		} else {
			a_spec->len = strlen(p);
			*pattern_next = p + a_spec->len;
		}
		a_spec->write_buf = zlog_spec_write_str;
		a_spec->gen_msg = zlog_spec_gen_msg_direct;
		a_spec->gen_path = zlog_spec_gen_path_direct;
		a_spec->gen_archive_path = zlog_spec_gen_archive_path_direct;
	}

	zlog_spec_profile(a_spec, ZC_DEBUG);
	return a_spec;
err:
	zlog_spec_del(a_spec);
	return NULL;
}

/*******************************************************************************/
int zlog_spec_gen_list(zc_sds pattern, zc_arraylist_t **list)
{
	/* replace any environment variables like %E(HOME) */
	rc = zc_str_replace_env(a_rule->record_path, sizeof(a_rule->record_path));
	if (rc) {
		zc_error("zc_str_replace_env fail");
		goto err;
	}

	/* try to figure out if the log file path is dynamic or static */
	if (strchr(a_rule->record_path, '%') == NULL) {
		a_rule->output = zlog_rule_output_static_record;
	} else {
		zlog_spec_t *a_spec;

		a_rule->output = zlog_rule_output_dynamic_record;

		a_rule->dynamic_specs = zc_arraylist_new((zc_arraylist_del_fn)zlog_spec_del);
		if (!(a_rule->dynamic_specs)) {
			zc_error("zc_arraylist_new fail");
			goto err;
		}
		for (p = a_rule->record_path; *p != '\0'; p = q) {
			a_spec = zlog_spec_new(p, &q, time_cache_count);
			if (!a_spec) {
				zc_error("zlog_spec_new fail");
				goto err;
			}

			rc = zc_arraylist_add(a_rule->dynamic_specs, a_spec);
			if (rc) {
				zlog_spec_del(a_spec);
				zc_error("zc_arraylist_add fail");
				goto err;
			}
		}
	}
}
