/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
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
#include <ctype.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rule.h"
#include "format.h"
#include "buf.h"
#include "thread.h"
#include "priority.h"
#include "rotater.h"
#include "spec.h"

#include "zc_defs.h"
/*******************************************************************************/
static void xlog_rule_debug(xlog_rule_t * a_rule);
/*******************************************************************************/
static int xlog_rule_output_static_file_single(xlog_rule_t * a_rule, xlog_thread_t * a_thread)
{
	int rc = 0;
	int nobj = 0;
	char *msg;
	size_t msg_len;

	rc = xlog_format_gen_msg(a_rule->format, a_thread);
	if (rc) {
		zc_error("xlog_format_gen_msg fail");
		return -1;
	}

	msg = a_thread->msg_buf->start;
	msg_len = a_thread->msg_buf->end - a_thread->msg_buf->start;

	nobj = fwrite(msg, msg_len, 1, a_rule->static_file_stream);
	if (nobj < 1) {
		zc_error("fwrite error, errno[%d]", errno);
		return -1;
	}

	return 0;
}

static int xlog_rule_output_static_file_rotate(xlog_rule_t * a_rule, xlog_thread_t * a_thread)
{
	int rc = 0;
	int fd;
	ssize_t nwrite = 0;
	char *msg;
	size_t msg_len;

	rc = xlog_format_gen_msg(a_rule->format, a_thread);
	if (rc) {
		zc_error("xlog_format_gen_msg fail");
		return -1;
	}

	msg = a_thread->msg_buf->start;
	msg_len = a_thread->msg_buf->end - a_thread->msg_buf->start;

	fd = open(a_rule->file_path, O_WRONLY | O_APPEND | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd < 0) {
		zc_error("open file[%s] fail, errno[%d]", a_rule->file_path, errno);
		return -1;
	}

	nwrite = write(fd, msg, msg_len);
	if (nwrite < 0) {
		zc_error("write fail, errno[%d]", errno);
		close(fd);
		return -1;
	}
	close(fd);

	rc = xlog_rotater_rotate(&xlog_env_rotater, a_rule->file_path, a_rule->file_maxsize, msg_len);
	if (rc) {
		zc_error("xlog_rotater_rotate fail");
		return -1;
	}

	return 0;
}

/* return 0	success
 * return !=0	fail
 */
static int xlog_rule_gen_path(xlog_rule_t * a_rule, xlog_thread_t *a_thread, char **file_path)
{
	int rc;
	int i;
	xlog_spec_t *a_spec;

	xlog_buf_restart(a_thread->path_buf);

	for (i = 0; i < zc_arraylist_len(a_rule->dynamic_file_specs); i++) {
		a_spec = (xlog_spec_t *) zc_arraylist_get(a_rule->dynamic_file_specs, i);
		if (!a_spec) {
			zc_error("get null spec");
			return -1;
		}

		rc = xlog_spec_gen_path(a_spec, a_thread);
		if (rc) {
			zc_error("xlog_spec_gen_path fail");
			return -1;
		}
	}

	*file_path = a_thread->path_buf->start;
	return 0;
}


static int xlog_rule_output_dynamic_file_single(xlog_rule_t * a_rule, xlog_thread_t * a_thread)
{
	int rc = 0;
	ssize_t nwrite = 0;
	char *msg;
	size_t msg_len;
	char *file_path;
	int fd;

	zc_assert(a_rule, -1);
	zc_assert(a_thread, -1);

	rc = xlog_rule_gen_path(a_rule, a_thread, &file_path);
	if (rc) {
		zc_error("xlog_rule_gen_path fail");
		return -1;
	}

	rc = xlog_format_gen_msg(a_rule->format, a_thread);
	if (rc) {
		zc_error("xlog_format_output fail");
		return -1;
	}

	msg = a_thread->msg_buf->start;
	msg_len = a_thread->msg_buf->end - a_thread->msg_buf->start;

	fd = open(file_path, O_WRONLY | O_APPEND | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd < 0) {
		zc_error("open file[%s] fail, errno[%d]", a_rule->file_path, errno);
		return -1;
	}

	nwrite = write(fd, msg, msg_len);
	if (nwrite < 0) {
		zc_error("write fail, errno[%d]", errno);
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

static int xlog_rule_output_dynamic_file_rotate(xlog_rule_t * a_rule, xlog_thread_t * a_thread)
{
	int rc = 0;
	ssize_t nwrite = 0;
	char *msg;
	size_t msg_len;
	char *file_path;
	int fd;

	zc_assert(a_rule, -1);
	zc_assert(a_thread, -1);

	rc = xlog_rule_gen_path(a_rule, a_thread, &file_path);
	if (rc) {
		zc_error("xlog_rule_gen_path fail");
		return -1;
	}

	rc = xlog_format_gen_msg(a_rule->format, a_thread);
	if (rc) {
		zc_error("xlog_format_output fail");
		return -1;
	}

	msg = a_thread->msg_buf->start;
	msg_len = a_thread->msg_buf->end - a_thread->msg_buf->start;

	fd = open(file_path, O_WRONLY|O_APPEND|O_CREAT,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if (fd < 0) {
		zc_error("open file[%s] fail, errno[%d]", a_rule->file_path, errno);
		return -1;
	}

	nwrite = write(fd, msg, msg_len);
	if (nwrite < 0) {
		zc_error("write fail, errno[%d]", errno);
		close(fd);
		return -1;
	}
	close(fd);

	rc = xlog_rotater_rotate(&xlog_env_rotater, file_path, a_rule->file_maxsize, msg_len);
	if (rc) {
		zc_error("xlog_rotater_rotate fail");
		return -1;
	}

	return 0;
}

static int xlog_rule_output_syslog(xlog_rule_t * a_rule, xlog_thread_t * a_thread)
{
	int rc = 0;
	char *msg;
	size_t msg_len;

	rc = xlog_format_gen_msg(a_rule->format, a_thread);
	if (rc) {
		zc_error("xlog_format_gen_msg fail");
		return -1;
	}

	msg = a_thread->msg_buf->start;
	msg_len = a_thread->msg_buf->end - a_thread->msg_buf->start;

	syslog(xlog_priority_to_syslog(a_thread->event->priority), "%s", msg);
	return 0;
}

static int xlog_rule_output_stdout(xlog_rule_t * a_rule, xlog_thread_t * a_thread)
{
	int rc = 0;
	int nobj = 0;
	char *msg;
	size_t msg_len;

	rc = xlog_format_gen_msg(a_rule->format, a_thread);
	if (rc) {
		zc_error("xlog_format_gen_msg fail");
		return -1;
	}

	msg = a_thread->msg_buf->start;
	msg_len = a_thread->msg_buf->end - a_thread->msg_buf->start;

	nobj = fwrite(msg, msg_len, 1, stdout);
	if (nobj < 1) {
		zc_error("fwrite error, errno[%d]", errno);
		return -1;
	}

	return 0;
}

static int xlog_rule_output_stderr(xlog_rule_t * a_rule, xlog_thread_t * a_thread)
{
	int rc = 0;
	int nobj = 0;
	char *msg;
	size_t msg_len;

	rc = xlog_format_gen_msg(a_rule->format, a_thread);
	if (rc) {
		zc_error("xlog_format_gen_msg fail");
		return -1;
	}

	msg = a_thread->msg_buf->start;
	msg_len = a_thread->msg_buf->end - a_thread->msg_buf->start;

	nobj = fwrite(msg, msg_len, 1, stderr);
	if (nobj < 1) {
		zc_error("fwrite error, errno[%d]", errno);
		return -1;
	}

	return 0;
}

/*******************************************************************************/
static int syslog_facility_strtoi(char *facility)
{
	zc_assert(facility, LOG_USER);

	if (STRICMP(facility, ==, "LOG_LOCAL0"))
		return LOG_LOCAL0;
	if (STRICMP(facility, ==, "LOG_LOCAL1"))
		return LOG_LOCAL1;
	if (STRICMP(facility, ==, "LOG_LOCAL2"))
		return LOG_LOCAL2;
	if (STRICMP(facility, ==, "LOG_LOCAL3"))
		return LOG_LOCAL3;
	if (STRICMP(facility, ==, "LOG_LOCAL4"))
		return LOG_LOCAL4;
	if (STRICMP(facility, ==, "LOG_LOCAL5"))
		return LOG_LOCAL5;
	if (STRICMP(facility, ==, "LOG_LOCAL6"))
		return LOG_LOCAL6;
	if (STRICMP(facility, ==, "LOG_LOCAL7"))
		return LOG_LOCAL7;

	return LOG_USER;
}

xlog_rule_t *xlog_rule_new(zc_arraylist_t *formats, char *line, long line_len)
{
	int rc = 0;
	int nscan = 0;
	int nread = 0;
	xlog_rule_t *a_rule;

	char selector[MAXLEN_CFG_LINE + 1];
	char category[MAXLEN_CFG_LINE + 1];
	char priority[MAXLEN_CFG_LINE + 1];

	char *action;
	char output[MAXLEN_CFG_LINE + 1];
	char format_name[MAXLEN_CFG_LINE + 1];
	char file_path[MAXLEN_CFG_LINE + 1];
	char file_maxsize[MAXLEN_CFG_LINE + 1];

	char *p;
	char *q;
	int len;

	zc_assert(formats, NULL);
	zc_assert(line, NULL);
	zc_assert(line_len, NULL);

	a_rule = calloc(1, sizeof(xlog_rule_t));
	if (!a_rule) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	memset(&selector, 0x00, sizeof(selector));
	nscan = sscanf(line, "%s %n", selector, &nread);
	if (nscan != 1) {
		zc_error("sscanf [%s] fail, selector", line);
		rc = -1;
		goto xlog_rule_new_exit;
	}
	action = line + nread;

	/* line         [f.INFO "$H/log/aa.log", 20MB; MyTemplate]
	 * selector     [f.INFO]
	 * action       ["$H/log/aa.log", 20MB; MyTemplate]
	 */

	memset(category, 0x00, sizeof(category));
	memset(priority, 0x00, sizeof(priority));
	nscan = sscanf(selector, " %[^.].%s", category, priority);
	if (nscan != 2) {
		zc_error("sscanf [%s] fail, category or priority is null", selector);
		rc = -1;
		goto xlog_rule_new_exit;
	}

	/*
	 * selector     [f.INFO]
	 * category     [f]
	 * priority     [.INFO]
	 */

	/* check and set category */
	for (p = category; *p != '\0'; p++) {
		if ((!isalnum(*p)) && (*p != '_') && (*p != '*')) {
			zc_error("category name[%s] is not alpha, digit or _", category);
			rc = -1;
			goto xlog_rule_new_exit;
		}
	}

	/* as one line can't be longer than MAXLEN_CFG_LINE, same as category */
	strcpy(a_rule->category, category);

	/* check and set priority */
	switch (priority[1]) {
	case '=':
	case '!':
		a_rule->compare_char = priority[1];
		p = priority + 2;
		a_rule->priority = xlog_priority_strtoi(p);
		break;
	case '*':
		a_rule->compare_char = priority[1];
		a_rule->priority = 0;
		break;
	default:
		a_rule->compare_char = '.';
		p = priority;
		a_rule->priority = xlog_priority_strtoi(p);
		break;
	}

	memset(output, 0x00, sizeof(output));
	memset(format_name, 0x00, sizeof(format_name));
	nscan = sscanf(action, " %[^;];%s", output, format_name);
	if (nscan < 1) {
		zc_error("sscanf [%s] fail", action);
		rc = -1;
		goto xlog_rule_new_exit;
	}

	/* action               ["$H/log/aa.log", 20MB ; MyTemplate]
	 * output               ["$H/log/aa.log", 20MB ]
	 * format               [MyTemplate]
	 */

	/* check and get format */
	if (STRCMP(format_name, ==, "")) {
		zc_debug("no format specified, use default");
		a_rule->format = zc_arraylist_get(formats, 0);
		if (!a_rule->format) {
			zc_error("zc_arraylist_get fail");
			rc = -1;
			goto xlog_rule_new_exit;
		}
	} else {
		int i;
		int find_flag = 0;
		xlog_format_t *a_format;

		for (i = 0, find_flag = 0; i < zc_arraylist_len(formats); i++) {
			a_format = (xlog_format_t *) zc_arraylist_get(formats, i);
			if (!a_format) {
				zc_error("a_format is null");
				rc = -1;
				goto xlog_rule_new_exit;
			}
			if (STRCMP(a_format->name, ==, format_name)) {
				a_rule->format = a_format;
				find_flag = 1;
				break;
			}
		}

		if (!find_flag) {
			zc_error("in conf file can't find format[%s], pls check", format_name);
			rc = -1;
			goto xlog_rule_new_exit;
		}
	}

	memset(file_path, 0x00, sizeof(file_path));
	memset(file_maxsize, 0x00, sizeof(file_maxsize));
	nscan = sscanf(output, " %[^,],%s", file_path, file_maxsize);
	if (nscan < 1) {
		zc_error("sscanf [%s] fail", action);
		rc = -1;
		goto xlog_rule_new_exit;
	}

	/* output               ["$(HOME)/log/aa.log" , 20MB ]  [>syslog , LOG_LOCAL0 ]
	 * file_path            ["$(HOME)/log/aa.log" ]         [>syslog ]
	 * file_maxsize         [20MB]                          [LOG_LOCAL0]
	 */

	switch (file_path[0]) {
	case '"':
		a_rule->file_maxsize = zc_parse_byte_size(file_maxsize);

		p = strrchr(file_path + 1, '"');
		if (!p) {
			zc_error("matching \" not found in conf line[%s]", output);
			rc = -1;
			goto xlog_rule_new_exit;
		}
		len = p - file_path - 1;
		if (len > sizeof(a_rule->file_path) - 1) {
			zc_error("file_path[%s] too long > %ld", sizeof(a_rule->file_path) - 1);
			rc = -1;
			goto xlog_rule_new_exit;
		}
		strncpy(a_rule->file_path, file_path + 1, len);

		/* replace any environment variables like $E(HOME) */
		rc = zc_str_replace_env(a_rule->file_path, sizeof(a_rule->file_path));
		if (rc) {
			zc_error("zc_str_replace_env fail");
			rc = -1;
			goto xlog_rule_new_exit;
		}

		/* try to guess if the log file path is dynamic or static */
		if (strchr(a_rule->file_path, '$') == NULL) {
			if (a_rule->file_maxsize <= 0 ) {
				a_rule->static_file_stream = fopen(a_rule->file_path, "a");
				if (!a_rule->static_file_stream) {
					zc_error("fopen[%s] fail, errno[%d]", a_rule->file_path, errno);
					rc = -1;
					goto xlog_rule_new_exit;
				}
				a_rule->output_fn = xlog_rule_output_static_file_single;
			} else {
				/* as rotate, so need to reopen everytime */
				a_rule->output_fn = xlog_rule_output_static_file_rotate;
			}
		} else {
			xlog_spec_t *a_spec;

			a_rule->dynamic_file_specs = zc_arraylist_new((zc_arraylist_del_fn) xlog_spec_del);
			if (!(a_rule->dynamic_file_specs)) {
				zc_error("zc_arraylist_new fail");
				rc = -1;
				goto xlog_rule_new_exit;
			}
			for (p = a_rule->file_path; *p != '\0'; p = q) {
				a_spec = xlog_spec_new(p, &q);
				if (rc) {
					zc_error("xlog_spec_new fail");
					rc = -1;
					goto xlog_rule_new_exit;
				}

				rc = zc_arraylist_add(a_rule->dynamic_file_specs, a_spec);
				if (rc) {
					zc_error("zc_arraylist_add fail");
					rc = -1;
					goto xlog_rule_new_exit;
				}
			}

			if (a_rule->file_maxsize <= 0 ) {
				a_rule->output_fn = xlog_rule_output_dynamic_file_single;
			} else {
				a_rule->output_fn = xlog_rule_output_dynamic_file_rotate;
			}
		}

		break;
	case '>':
		if (STRNCMP(file_path + 1, ==, "syslog", 6)) {
			a_rule->syslog_facility = syslog_facility_strtoi(file_maxsize);
			a_rule->output_fn = xlog_rule_output_syslog;
			openlog(NULL, LOG_NDELAY|LOG_NOWAIT|LOG_PID, a_rule->syslog_facility);
		} else if (STRNCMP(file_path + 1, ==, "stdout", 6)) {
			a_rule->output_fn = xlog_rule_output_stdout;
		} else if (STRNCMP(file_path + 1, ==, "stderr", 6)) {
			a_rule->output_fn = xlog_rule_output_stderr;
		} else {
			zc_error("[%s]the string after is not syslog, stdout or stderr", output);
		}
		break;
	default:
		printf("the 1st char[%c] of file_path[%s] is wrong", file_path[0], file_path);
		rc = -1;
		goto xlog_rule_new_exit;
	}

      xlog_rule_new_exit:

	if (rc) {
		xlog_rule_del(a_rule);
		return NULL;
	} else {
		xlog_rule_debug(a_rule);
		return a_rule;
	}
}

void xlog_rule_del(xlog_rule_t * a_rule)
{
	zc_assert(a_rule,);

	if (a_rule->dynamic_file_specs) {
		zc_arraylist_del(a_rule->dynamic_file_specs);
		a_rule->dynamic_file_specs = NULL;
	}
	if (a_rule->static_file_stream) {
		fclose(a_rule->static_file_stream);
		a_rule->static_file_stream = NULL;
	}
	zc_debug("free a_rule at[%p]", a_rule);
	free(a_rule);

	return;
}

/*******************************************************************************/
int xlog_rule_output(xlog_rule_t * a_rule,  xlog_thread_t * a_thread)
{
	zc_assert(a_rule, -1);
	zc_assert(a_thread, -1);

	return a_rule->output_fn(a_rule, a_thread);
}

/*******************************************************************************/
int xlog_rule_match_category(xlog_rule_t * a_rule, char *category)
{
	zc_assert(a_rule, -1);
	zc_assert(category, -1);

	if (STRCMP(a_rule->category, ==, "*")) {
		/* '*' match anything, so go on */
		return 1;
	} else if (STRCMP(a_rule->category, ==, category)) {
		/* accurate compare */
		return 1;
	} else {
		/* aa_ match aa_xx & aa, but not match aa1_xx */
		size_t len;
		len = strlen(a_rule->category);

		if (a_rule->category[len - 1] == '_') {
			if (strlen(category) == len - 1) {
				len--;
			}

			if (STRNCMP(a_rule->category, ==, category, len)) {
				return 1;
			}
		}
	}

	return 0;
}

/*******************************************************************************/
static void xlog_rule_debug(xlog_rule_t * a_rule)
{
	zc_debug("rule:[%p][%s%c%d]-[%s,%ld][%p]", a_rule,
		a_rule->category, a_rule->compare_char, a_rule->priority,
		a_rule->file_path, a_rule->file_maxsize, a_rule->format);
	return;
}

void xlog_rule_profile(xlog_rule_t * a_rule)
{
	zc_assert(a_rule, );
	zc_error("rule:[%p][%s%c%d]-[%s,%ld][%p]", a_rule,
		a_rule->category, a_rule->compare_char, a_rule->priority,
		a_rule->file_path, a_rule->file_maxsize, a_rule->format);
	return;
}

/*******************************************************************************/
