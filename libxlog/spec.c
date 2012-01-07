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
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "spec.h"
#include "priority.h"
#include "zc_defs.h"

/*******************************************************************************/
static void xlog_spec_debug(xlog_spec_t * a_spec);

/*******************************************************************************/
static int xlog_spec_gen_time_direct(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	/* only when need fetch time, do it once */
	if (!a_thread->event->time_stamp.tv_sec) {
		gettimeofday(&(a_thread->event->time_stamp), NULL);
		localtime_r(&(a_thread->event->time_stamp.tv_sec), &(a_thread->event->local_time));
		sprintf(a_thread->event->us, "%6.6ld", (long)a_thread->event->time_stamp.tv_usec);
	}

	rc = xlog_buf_strftime(a_buf, a_spec->time_fmt, a_spec->time_len, &(a_thread->event->local_time));
	if (rc) {
		zc_error("xlog_buf_strftime maybe fail or overflow");
		return rc;
	}

	return 0;
}

static int xlog_spec_gen_time_msus(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int i;
	int rc;

	/* only when need fetch time, do it once */
	if (!a_thread->event->time_stamp.tv_sec) {
		gettimeofday(&(a_thread->event->time_stamp), NULL);
		localtime_r(&(a_thread->event->time_stamp.tv_sec), &(a_thread->event->local_time));
		sprintf(a_thread->event->us, "%6.6ld", (long)a_thread->event->time_stamp.tv_usec);
	}

	/* replace real microsec and millisec here */
	strcpy(a_thread->event->time_fmt_msus, a_spec->time_fmt);
	for (i = 0; i < a_spec->ms_count; i++) {
		memcpy(a_thread->event->time_fmt_msus + 
			a_spec->ms_offset[i], a_thread->event->us, 3);
	}

	for (i = 0; i < a_spec->us_count; i++) {
		memcpy(a_thread->event->time_fmt_msus + 
			a_spec->us_offset[i], a_thread->event->us, 6);
	}

	rc = xlog_buf_strftime(a_buf, a_thread->event->time_fmt_msus,
		a_spec->time_len, &(a_thread->event->local_time));
	if (rc) {
		zc_error("xlog_buf_strftime maybe fail or overflow");
		return rc;
	}

	return 0;
}

static int xlog_spec_gen_mdc(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;
	xlog_mdc_kv_t *a_mdc_kv;

	a_mdc_kv = xlog_mdc_get_kv(a_thread->mdc, a_spec->mdc_key);
	if (!a_mdc_kv) {
		zc_error("xlog_mdc_get_kv key[%s] fail", a_spec->mdc_key);
		return 0;
	}

	rc = xlog_buf_append(a_buf, a_mdc_kv->value, a_mdc_kv->value_len);
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}


static int xlog_spec_gen_str(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	rc = xlog_buf_append(a_buf, a_spec->str, a_spec->len);
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_category(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	
	int rc;

	rc = xlog_buf_append(a_buf, a_thread->event->category_name, *(a_thread->event->category_name_len));
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_srcfile(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	rc = xlog_buf_append(a_buf, a_thread->event->file, strlen(a_thread->event->file));
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_srcline(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	rc = xlog_buf_printf(a_buf, "%ld" , a_thread->event->line);
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_hostname(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	rc = xlog_buf_append(a_buf, a_thread->event->host_name, a_thread->event->host_name_len);
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}


static int xlog_spec_gen_newline(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	rc = xlog_buf_append(a_buf, FILE_NEWLINE, FILE_NEWLINE_LEN);
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_dollar(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	rc = xlog_buf_append(a_buf, "$", 1);
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_pid(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	if (a_thread->event->pid == 0) {
		a_thread->event->pid = getpid();
	}

	rc = xlog_buf_printf(a_buf, "%d", (int) a_thread->event->pid);
	if (rc) {
		zc_error("xlog_buf_printf maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_tid(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;

	/* don't need to get tid again, as tmap_new_thread fetch it already */

	rc = xlog_buf_printf(a_buf, "%ld", (long) a_thread->event->tid);
	if (rc) {
		zc_error("xlog_buf_printf maybe fail or overflow");
		return rc;
	}
	return 0;
}

static int xlog_spec_gen_priority(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc;
	char *priority_str;

	/* don't need to get tid again, as tmap_new_thread fetch it already */
	priority_str = xlog_priority_itostr(a_thread->event->priority);

	rc = xlog_buf_append(a_buf, priority_str, strlen(priority_str));
	if (rc) {
		zc_error("xlog_buf_append maybe fail or overflow");
		return rc;
	}
	return 0;
}


static int xlog_spec_gen_usrmsg(xlog_spec_t * a_spec, xlog_thread_t * a_thread, xlog_buf_t * a_buf)
{
	int rc = 0;

	static char xlog_hex_head[] =
	"             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF";

	if (a_thread->event->generate_cmd == XLOG_FMT) {

		if (a_thread->event->str_format == NULL) {
			rc = xlog_buf_printf(a_buf, "format=(null)");
			if (rc) {
				zc_error("xlog_buf_printf maybe fail or overflow");
				return rc;
			}
			return 0;
		} else {
			rc = xlog_buf_vprintf(a_buf, a_thread->event->str_format, a_thread->event->str_args);
			if (rc) {
				zc_error("xlog_buf_vprintf maybe fail or overflow");
				return rc;
			}
			return 0;
		}
	} else if (a_thread->event->generate_cmd == XLOG_HEX) {
		long line_offset;
		long byte_offset;

		/* thread buf start == null or len <= 0 */
		if (a_thread->event->hex_buf == NULL) {
			rc = xlog_buf_printf(a_buf, "hex_buf=(null)");
			goto xlog_hex_exit;
		} else if (a_thread->event->hex_buf_len <= 0) {
			rc = xlog_buf_printf(a_buf, "(hex_buf_len=%ld) <= 0", a_thread->event->hex_buf_len);
			goto xlog_hex_exit;
		}

		rc = xlog_buf_printf(a_buf, "hex_buf_len=[%ld]\n", a_thread->event->hex_buf_len);
		if (rc) {
			goto xlog_hex_exit;
		}

		rc = xlog_buf_append(a_buf, xlog_hex_head, strlen(xlog_hex_head));
		if (rc) {
			goto xlog_hex_exit;
		}

		line_offset = 0;
		byte_offset = 0;

		while (1) {
			unsigned char c;

			rc = xlog_buf_append(a_buf, "\n", 1);
			if (rc) {
				goto xlog_hex_exit;
			}

			rc = xlog_buf_printf(a_buf, "%010.10ld   ", line_offset + 1);
			if (rc) {
				goto xlog_hex_exit;
			}

			for (byte_offset = 0; byte_offset < 16; byte_offset++) {
				if (line_offset * 16 + byte_offset < a_thread->event->hex_buf_len) {
					c = *((unsigned char *)a_thread->event->hex_buf + line_offset * 16 + byte_offset);
					rc = xlog_buf_printf(a_buf, "%02x ", c);
					if (rc) {
						goto xlog_hex_exit;
					}
				} else {
					rc = xlog_buf_append(a_buf, "   ", 3);
					if (rc) {
						goto xlog_hex_exit;
					}
				}
			}

			rc = xlog_buf_append(a_buf, "  ", 2);
			if (rc) {
				goto xlog_hex_exit;
			}

			for (byte_offset = 0; byte_offset < 16; byte_offset++) {
				if (line_offset * 16 + byte_offset < a_thread->event->hex_buf_len) {
					c = *((unsigned char *)a_thread->event->hex_buf + line_offset * 16 + byte_offset);
					if (c >= 32 && c <= 126) {
						rc = xlog_buf_printf(a_buf, "%c", c);
						if (rc) {
							goto xlog_hex_exit;
						}
					} else {
						rc = xlog_buf_append(a_buf, ".", 1);
						if (rc) {
							goto xlog_hex_exit;
						}
					}
				} else {
					rc = xlog_buf_append(a_buf, " ", 1);
					if (rc) {
						goto xlog_hex_exit;
					}
				}
			}

			if (line_offset * 16 + byte_offset >= a_thread->event->hex_buf_len) {
				break;
			}

			line_offset++;
		}

	      xlog_hex_exit:
		if (rc < 0) {
			zc_error("generate hex msg fail");
			return -1;
		} else if (rc > 0) {
			zc_error("generate hex msg, buf is full");
			return 1;
		}

		return 0;
	}

	return 0;
}
/*******************************************************************************/
int xlog_spec_gen_msg(xlog_spec_t * a_spec, xlog_thread_t *a_thread)
{
	zc_assert(a_spec, -1);
	zc_assert(a_thread, -1);
	return a_spec->gen_msg(a_spec, a_thread);
}

static int xlog_spec_gen_msg_direct(xlog_spec_t * a_spec, xlog_thread_t *a_thread)
{
	int rc = 0;

	/* no need to reprint $1.2d here */
	rc = a_spec->gen_buf(a_spec, a_thread, a_thread->msg_buf);
	if (rc < 0) {
		zc_error("a_spec->gen_buf fail");
		return -1;
	} else if (rc > 0) {
		/* buf is full, make out loop stop */
		return 1;
	}
	return 0;
}

static int xlog_spec_gen_msg_reformat(xlog_spec_t * a_spec, xlog_thread_t *a_thread)
{
	int rc = 0;

	xlog_buf_restart(a_thread->pre_msg_buf);

	rc = a_spec->gen_buf(a_spec, a_thread, a_thread->pre_msg_buf);
	if (rc < 0) {
		zc_error("a_spec->gen_buf fail");
		return -1;
	} else if (rc > 0) {
		/* buf is full, try printf */
	}

	/** @todo use own alignment buf func here, for speed up */
	/* now process $1.2 here */
	rc = xlog_buf_printf(a_thread->msg_buf, a_spec->print_fmt, a_thread->pre_msg_buf->start);
	if (rc < 0) {
		zc_error("xlog_buf_printf fail");
		return -1;
	} else if (rc > 0) {
		/* buf is full, make out loop stop */
		return 1;
	}

	return 0;
}

/*******************************************************************************/
int xlog_spec_gen_path(xlog_spec_t * a_spec, xlog_thread_t *a_thread)
{
	zc_assert(a_spec, -1);
	zc_assert(a_thread, -1);
	return a_spec->gen_path(a_spec, a_thread);
}

static int xlog_spec_gen_path_direct(xlog_spec_t * a_spec, xlog_thread_t *a_thread)
{
	int rc = 0;

	/* no need to reprint $1.2d here */
	rc = a_spec->gen_buf(a_spec, a_thread, a_thread->path_buf);
	if (rc < 0) {
		zc_error("a_spec->gen_buf fail");
		return -1;
	} else if (rc > 0) {
		/* buf is full, make out loop stop */
		return 1;
	}
	return 0;
}

static int xlog_spec_gen_path_reformat(xlog_spec_t * a_spec, xlog_thread_t *a_thread)
{
	int rc = 0;

	xlog_buf_restart(a_thread->pre_path_buf);

	rc = a_spec->gen_buf(a_spec, a_thread, a_thread->pre_path_buf);
	if (rc < 0) {
		zc_error("a_spec->gen_buf fail");
		return -1;
	} else if (rc > 0) {
		/* buf is full, try printf */
	}

	/** @todo use own alignment buf func here, for speed up */
	/* now process $1.2 here */
	rc = xlog_buf_printf(a_thread->path_buf, a_spec->print_fmt, a_thread->pre_path_buf->start);
	if (rc < 0) {
		zc_error("xlog_buf_printf fail");
		return -1;
	} else if (rc > 0) {
		/* buf is full, make out loop stop */
		return 1;
	}

	return 0;
}

/*******************************************************************************/
static int xlog_spec_parse_time_fmt(xlog_spec_t * a_spec)
{
	char a_time[3 * MAXLEN_CFG_LINE + 1];
	char a_time_fmt[3 * MAXLEN_CFG_LINE + 1];
	char b_time_fmt[3 * MAXLEN_CFG_LINE + 1];
	time_t a_tt;
	struct tm a_tm;
	char *p;
	char *q;

	memset(a_time_fmt, 0x00, sizeof(a_time_fmt));

	/* replace %us to UUUUUU and mark position, later replace to real  microseconds */
	a_spec->us_count = 0;
	q = a_spec->time_fmt;
	while (1) {
		p = strstr(q, "%us");
		if (!p) {
			strcat(a_time_fmt, q);
			break;
		}

		strncat(a_time_fmt, q, p - q);
		a_spec->us_offset[a_spec->us_count++] = strlen(a_time_fmt);;
		strcat(a_time_fmt, "UUUUUU");
		q = p + 3;
	}

	memset(b_time_fmt, 0x00, sizeof(a_time_fmt));

	/* replace %ms to MMM and mark position, later replace to real  milliseconds */
	/* as strlen(%ms) == strlen(MMM), so that will not change postion of us */
	a_spec->ms_count = 0;
	q = a_spec->time_fmt;
	q = a_time_fmt;
	while (1) {
		p = strstr(q, "%ms");
		if (!p) {
			strcat(b_time_fmt, q);
			break;
		}

		strncat(b_time_fmt, q, p - q);
		a_spec->ms_offset[a_spec->ms_count++] = strlen(b_time_fmt);;
		strcat(b_time_fmt, "MMM");
		q = p + 3;
	}

	if (strlen(b_time_fmt) > sizeof(a_spec->time_fmt) - 1) {
		zc_error("after replace, b_time_fmt[%s] is overflow", b_time_fmt);
		return -1;
	} else {
		strcpy(a_spec->time_fmt, b_time_fmt);
	}

	/* get time once, calc how long will be the time_string */
	time(&a_tt);
	localtime_r(&(a_tt), &(a_tm));

	memset(a_time, 0x00, sizeof(a_time));
	strftime(a_time, sizeof(a_time), a_spec->time_fmt, &a_tm);

	/* in real world the timelen may be longer than now */
	a_spec->time_len = strlen(a_time) + 10;
	return 0;
}

/* a spec may consist of
 * a const string: /home/bb
 * a string begin with $: $12.35d(%F %X,%l)
 */
xlog_spec_t * xlog_spec_new(char *pattern_start, char **pattern_next)
{
	int rc = 0;
	char *p;
	int nscan, nread;
	xlog_spec_t *a_spec;

	zc_assert(pattern_start, NULL);
	zc_assert(pattern_next, NULL);

	a_spec = calloc(1, sizeof(xlog_spec_t));
	if (!a_spec) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	a_spec->str = p = pattern_start;

	switch (*p) {
 	/* a string begin with $: $12.35d(%F %X,%l) */
	case '$':
		/* process width and precision char in $-12.35P */
		nscan = sscanf(p, "$%[.0-9-]%n", a_spec->print_fmt + 1, &nread);
		if (nscan == 1) {
			a_spec->print_fmt[0] = '%';
			a_spec->print_fmt[nread] = 's';
			a_spec->gen_msg = xlog_spec_gen_msg_reformat;
			a_spec->gen_path = xlog_spec_gen_path_reformat;
		} else {
			nread = 1;
			a_spec->gen_msg = xlog_spec_gen_msg_direct;
			a_spec->gen_path = xlog_spec_gen_path_direct;
		}

		p += nread;

		if (*p == 'd') {
			nscan = sscanf(p, "d(%[^)])%n", a_spec->time_fmt, &nread);
			if (nscan == 0) {
				nread = 0;
				if (STRNCMP(p, ==, "d()", 3)) {
					nread = 3;
				}
			}
			p += nread;
			if (*(p - 1) != ')') {
				zc_error("in string[%s] can't find match \')\'", a_spec->str);
				rc = -1;
				goto xlog_spec_init_exit;
			}

			rc = xlog_spec_parse_time_fmt(a_spec);
			if (rc) {
				zc_error("xlog_spec_parse_time_fmt fail");
				rc = -1;
				goto xlog_spec_init_exit;
			}

			*pattern_next = p;
			a_spec->len = p - a_spec->str;
			if (a_spec->ms_count > 0 || a_spec->us_count > 0) {
				a_spec->gen_buf = xlog_spec_gen_time_msus;
			} else {
				a_spec->gen_buf = xlog_spec_gen_time_direct;
			}
			break;
		}

		if (*p == 'M') {
			nscan = sscanf(p, "M(%[^)])%n", a_spec->mdc_key, &nread);
			if (nscan == 0) {
				nread = 0;
				if (STRNCMP(p, ==, "M()", 3)) {
					nread = 3;
				}
			}
			p += nread;
			if (*(p - 1) != ')') {
				zc_error("in string[%s] can't find match \')\'", a_spec->str);
				rc = -1;
				goto xlog_spec_init_exit;
			}

			*pattern_next = p;
			a_spec->len = p - a_spec->str;
			a_spec->gen_buf = xlog_spec_gen_mdc;
			break;
		}

		*pattern_next = p + 1;
		a_spec->len = p - a_spec->str + 1;

		switch (*p) {
		case 'c':
			a_spec->gen_buf = xlog_spec_gen_category;
			break;
		case 'F':
			a_spec->gen_buf = xlog_spec_gen_srcfile;
			break;
		case 'H':
			a_spec->gen_buf = xlog_spec_gen_hostname;
			break;
		case 'L':
			a_spec->gen_buf = xlog_spec_gen_srcline;
			break;
		case 'm':
			a_spec->gen_buf = xlog_spec_gen_usrmsg;
			break;
		case 'n':
			a_spec->gen_buf = xlog_spec_gen_newline;
			break;
		case 'p':
			a_spec->gen_buf = xlog_spec_gen_pid;
			break;
		case 'P':
			a_spec->gen_buf = xlog_spec_gen_priority;
			break;
		case 't':
			a_spec->gen_buf = xlog_spec_gen_tid;
			break;
		case '$':
			a_spec->gen_buf = xlog_spec_gen_dollar;
			break;
		default:
			zc_error("str[%s] in wrong format, p[%c]", a_spec->str, *p);
			rc = -1;
			goto xlog_spec_init_exit;
		}
		break;
 	/* a const string: /home/bb */
	default:
		*pattern_next = strchr(p, '$');
		if (*pattern_next) {
			a_spec->len = *pattern_next - p;
		} else {
			a_spec->len = strlen(p);
			*pattern_next = p + a_spec->len;
		}
		a_spec->gen_buf = xlog_spec_gen_str;
		a_spec->gen_msg = xlog_spec_gen_msg_direct;
	}


      xlog_spec_init_exit:
	if (rc) {
		xlog_spec_del(a_spec);
		return NULL;
	} else {
		xlog_spec_debug(a_spec);
		return a_spec;
	}
}

/*******************************************************************************/
void xlog_spec_del(xlog_spec_t * a_spec)
{
	zc_assert(a_spec,);

	free(a_spec);
	zc_debug("free a_spec at[%p]", a_spec);
}


/*******************************************************************************/
static void xlog_spec_debug(xlog_spec_t * a_spec)
{
	zc_debug("spec:[%p][%.*s][%s %d][%s]", a_spec,
		a_spec->len, a_spec->str,
		a_spec->time_fmt, a_spec->time_len,
		a_spec->print_fmt);
	return;
}

void xlog_spec_profile(xlog_spec_t * a_spec)
{
	zc_assert(a_spec, )
	zc_error("spec:[%p][%.*s][%s %d][%s]", a_spec,
		a_spec->len, a_spec->str,
		a_spec->time_fmt, a_spec->time_len,
		a_spec->print_fmt);
	return;
}
