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

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>

#include "zc_defs.h"
#include "event.h"

xlog_event_t * xlog_event_new(void)
{
	int rc = 0;
	xlog_event_t *a_event;

	a_event = calloc(1, sizeof(xlog_event_t));
	if (!a_event) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}


	/*
	 * at the xlog_init we gethostname,
	 * u don't always change your hostname, eh?
	 */
	rc = gethostname(a_event->host_name, sizeof(a_event->host_name) - 1);
	if (rc) {
		zc_error("gethostname fail, rc[%d], errno[%d]", rc, errno);
      		goto xlog_event_new_exit;
	}

	a_event->host_name_len = strlen(a_event->host_name);

	/* tid is bound to a_event
	 * as in whole lifecycle event persists
	 * even fork to oth pid, tid not change
	 */
	a_event->tid = pthread_self();
	zc_debug("first in thread[%ld]", a_event->tid);

      xlog_event_new_exit:
	if (rc) {
		xlog_event_del(a_event);
		return NULL;
	} else {
		return a_event;
	}
}

void xlog_event_del(xlog_event_t * a_event)
{
	zc_assert(a_event,);

	zc_debug("free a_event at[%p]", a_event);
	free(a_event);
	return;
}

void xlog_event_set(xlog_event_t * a_event,
		    char *category_name, size_t *category_name_len,
		    char *file, long line, int priority,
		    char *hex_buf, long hex_buf_len, char *str_format, va_list str_args, int generate_cmd)
{
	zc_assert(a_event, );

	/*
	 * category_name point to xlog_category_output's category.name
	 */
	a_event->category_name = category_name;
	a_event->category_name_len = category_name_len;

	a_event->file = file;
	a_event->line = line;
	a_event->priority = priority;

	a_event->generate_cmd = generate_cmd;
	switch (generate_cmd) {
	case XLOG_HEX:
		a_event->hex_buf = hex_buf;
		a_event->hex_buf_len = hex_buf_len;
		break;
	case XLOG_FMT:
		a_event->str_format = str_format;
		va_copy(a_event->str_args, str_args);
		break;
	default:
		break;
	}

	return;
}

#if 0
/* return 0     success
 * return <0    fail
 * return >0    because not enough space fail
 */
int xlog_event_generate_protomsg(xlog_event_t * a_event, xlog_buf_t * a_buf)
{
	int rc = 0;

	zc_assert(a_event, -1);
	zc_assert(a_buf, -1);

	if (a_event->generate_cmd == XLOG_FMT) {

		if (a_event->str_format == NULL) {
			rc = xlog_buf_printf(a_buf, "format=(null)");
			if (rc < 0) {
				zc_error("xlog_buf_printf fail");
				return -1;
			} else if (rc > 0) {
				zc_error("xlog_buf_printf, buf is full");
				return 1;
			} else {
				return 0;
			}
		}

		rc = xlog_buf_vprintf(a_buf, a_event->str_format, a_event->str_args);
		if (rc < 0) {
			zc_error("xlog_buf_vprintf fail");
			return -1;
		} else if (rc > 0) {
			zc_error("xlog_buf_vprintf, buf is full");
			return 1;
		} else {
			return 0;
		}
	} else if (a_event->generate_cmd == XLOG_HEX) {
		static char hex_head[] = "             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF";
		long line_offset;
		long byte_offset;

		/* process buf start == null or len <= 0 */
		if (a_event->hex_buf == NULL) {
			rc = xlog_buf_printf(a_buf, "hex_buf=(null)");
			goto xlog_hex_exit;
		} else if (a_event->hex_buf_len <= 0) {
			rc = xlog_buf_printf(a_buf, "(hex_buf_len=%ld) <= 0", a_event->hex_buf_len);
			goto xlog_hex_exit;
		}

		rc = xlog_buf_printf(a_buf, "hex_buf_len=[%ld]\n", a_event->hex_buf_len);
		if (rc) {
			goto xlog_hex_exit;
		}

		rc = xlog_buf_append(a_buf, hex_head, strlen(hex_head));
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
				if (line_offset * 16 + byte_offset < a_event->hex_buf_len) {
					c = *((unsigned char *)a_event->hex_buf + line_offset * 16 + byte_offset);
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
				if (line_offset * 16 + byte_offset < a_event->hex_buf_len) {
					c = *((unsigned char *)a_event->hex_buf + line_offset * 16 + byte_offset);
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

			if (line_offset * 16 + byte_offset >= a_event->hex_buf_len) {
				break;
			}

			line_offset++;
		}

	      xlog_hex_exit:
		if (rc < 0) {
			zc_error("generate hex protomsg fail");
			return -1;
		} else if (rc > 0) {
			zc_error("generate hex protomsg, buf is full");
			return 1;
		}

		return 0;
	}

	return 0;
}

#endif
