/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * The zlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The zlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the zlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#include "zlog.h"

#define CONF_STRING_CN \
""

#define CONF_STRING_EN \
"########\n"   \
"# Global setting begins with @.\n"   \
"# All global setting could be not written, for use default value.\n"   \
"# The full sytanx is:\n"   \
"#     @[key][n space or tab][value]\n"   \
"\n"   \
"### If ignore_error_format_rule is true,\n"   \
"#    zlog_init() will omit error syntax of formats and rules.\n"   \
"# Else if ignore_error_format_rule is false,\n"   \
"#    zlog_init() will check sytnax of all formats and rules strictly,\n"   \
"#    and any error will cause zlog_init() failed and return -1.\n"   \
"# Default:\n"   \
"# @ignore_error_format_rule          false\n"  \
"\n"   \
"### zlog allocates one log buffer in each thread.\n"   \
"# buf_size_min indicates size of buffer malloced at init time.\n"   \
"# While loging, if log content size > buf_size,\n"   \
"#     buffer will expand automaticly, till buf_size_max,\n"   \
"#     and log conten is truncated.\n"   \
"# If buf_size_max is 0, means buf_size is unlimited,\n"   \
"#     everytime buf_size = 2*buf_size,\n"   \
"#     till process use up all it's memory.\n"   \
"# Size can append with unit KB, MB or GB,\n"   \
"#     so [@buf_size_min 1024] equals [@buf_size_min 1KB].\n"   \
"# Default:\n"   \
"# @buf_size_min                      1024\n"  \
"# @buf_size_max                      2MB\n"  \
"\n"   \
"### rotate_lock_file is a lock file for\n"   \
"#     rotate a log safely between multi-process.\n"   \
"# zlog will create the file at zlog_init()\n"   \
"# Make sure your program has permission to create and read-write the file.\n"   \
"# If programs run by different users\n"   \
"#     who need to write and rotater a same log file,\n"   \
"#     make sure that each program has permission\n"   \
"#     to create and read-write the same lock file.\n"   \
"# Default:\n"   \
"# @rotate_lock_file                  /tmp/zlog.lock\n"  \
"\n"   \
"### default_format is used by rules without format\n"  \
"# That cause each rule without format specified, would yield output like this:\n"   \
"# 2012-02-14 17:03:12 INFO [3758:test_hello.c:39] hello, zlog\n"   \
"# You can set it to change the default behavior\n" \
"# The inner default format:\n"   \
"# @default_format                    \"%d(%F %T) %V [%p:%F:%L] %m%n\"\n"  \
"\n"   \
"### User defined levels here\n"   \
"# The inner default levels are\n"   \
"# @level                             DEBUG = 20, LOG_DEBUG\n"   \
"# @level                             INFO = 40, LOG_INFO\n"   \
"# @level                             NOTICE = 60, LOG_NOTICE\n"   \
"# @level                             WARN = 80, LOG_WARNING\n"   \
"# @level                             ERROR = 100, LOG_ERR\n"   \
"# @level                             FATAL = 120, LOG_ALERT\n"   \
"# @level                             UNKNOWN = 254, LOG_ERR\n"   \
"# The syntax is\n"   \
"# @level[n tabs or spaces][level string] = [level int], [syslog level, optional]\n"   \
"# level int should in [1,253], more larger, more important\n"   \
"# and suggest to be used with user-defined macros in source file\n"   \
"# see ~/test/test_level.c ~/test/test_level.h for example\n"   \
"# eg.\n"   \
"# @level                             TEST = 50\n"   \
"\n"   \
"########\n"   \
"# Format begins with &.\n"   \
"# The full sytanx is:\n"   \
"#     &[name][n tab or space]\"[conversion pattern]\"\n"   \
"\n"   \
"# Format name character must be in [a-Z][0-9][_]\n"   \
"\n"   \
"########\n"   \
"# Rule begins with nothing.\n"   \
"# The mainly sytanx is:\n"   \
"# [selector][n tab or space][action]\n"   \
"\n"   \
"# *.*                     >stdout\n"   \
"# [selector] = [category].[!=, optional][level]\n"   \
"#     [category] should be\n"   \
"#         *, matches all category\n"   \
"#         !, matches category that has no rule matched yet\n"   \
"#         normal category, not end with '_', accurate match.\n"   \
"#             eg. aa_bb matches zt = zlog_get_category(\"aa_bb\")\n"   \
"#         super-category, end with '_', match super-category and sub-categories\n"   \
"#             eg. aa_ matches zt1 = zlog_get_category(\"aa\"), zt2 = zlog_get_category(\"aa_bb\"),\n"   \
"#                             zt3 = zlog_get_category(\"aa_bb_cc\")...\n"   \
"#     [level] should be zlog inner default levels, or user-defined levels\n" \
"#         aa.debug, soucre log action >= debug\n" \
"#         aa.!debug, source log action != debug\n" \
"#         aa.=debug, source log action == debug\n" \
"# [action] = [output], [file size limitation,optional]; [format name, optional]\n"   \
"#     [output] can be\n"   \
"#         >stdout, to posix stranded output\n"   \
"#         >stderr, to posix stranded error\n"   \
"#         >syslog, to syslog, must flow syslog facility\n"   \
"#         \"file path\", can be absolute file path\n"   \
"#             or relative file path with conversion pattern.\n"    \
"#     [file size limitation]\n"  \
"#         if [output] is syslog, then here is syslog facility, \n"  \
"#            must be LOG_LOCAL[0-7] or LOG_USER\n"   \
"#         else it controls file size in bytes, and can be appended with kb,mb,gb \n"   \
"#            eg. 2048K, 1M, 10mb\n"   \
"#     [format name]\n"   \
"#         the format mentioned above, or the default format if not set\n"  \
"#\n"   \
"######## Appendix, Conversion Character\n"   \
"# c    category name                      aa_bb\n"  \
"# d    data & time %d(strftime format)    12-01 17:17:42\n"   \
"# E    environment variable %E(HOME)      /home/test\n"   \
"# F    file name, __FILE__                test_hello.c\n"   \
"# f    strip file name, __FILE__          test_hello.c\n"   \
"# H    host name, gethostname()           zlog-dev\n"   \
"# L    line number, __LINE__              123\n"   \
"# m    user message                       hello, zlog\n"   \
"# M    mdc, %M(key)                       value\n"   \
"# n    newline                            \\n\n"   \
"# p    process id, getpid()               13423\n"   \
"# V    log level, capital                 INFO\n"   \
"# v    log level, lowercase               info\n"   \
"# t    thread id, pthread_self            12343\n"   \
"# %%   a single percent sign              %\n"   \
"# %[other char]   wrong syntax, will cause zlog_init() fail\n"  \
"#\n"  \
"######## Appendix, tools\n"   \
"# zlog-chk-conf [conf file]\n"  \
"# zlog-gen-conf [conf file]\n"

#define CONF_STRING \
"# @ignore_error_format_rule          false\n"  \
"# @buf_size_min                      1024\n"  \
"# @buf_size_max                      2MB\n"  \
"# @rotate_lock_file                  /tmp/zlog.lock\n"  \
"# @default_format                    \"%d(%F %T) %V [%p:%F:%L] %m%n\"\n"  \
"# @level                             TRACE = 10, LOG_DEBUG\n"   \
"\n"  \
"\n"  \
"# *.*                     >stdout\n"  \
"# !.*                     \"/var/log/zlog.nomatch.log\"\n" 

int main(int argc, char *argv[])
{
	int rc = 0;
	int op;
	int comment = 0;
	int nwrite;
	char file_name[1024 + 1];
	zlog_category_t *zt;
	FILE *fp = NULL; 

	static const char *help = 
		"Useage: zlog-gen-conf [conf filename]...\n"
		"If no filename is specified, use zlog.conf as default\n"
		"\t-c \tChinese comment(UTF-8, if envrionment is GBK, use iconv transform)\n"
		"\t-e \tEnligsh comment\n"
		"\t-h,\tshow help message\n";

	while((op = getopt(argc, argv, "eh")) > 0) {
		if (op == 'h') {
			puts(help);
			return 0;
		} else if (op == 'c') {
			comment = 1;
		} else if (op == 'e') {
			comment = 2;
		}
	}

	argc -= optind;
	argv += optind;

	setenv("ZLOG_PROFILE_ERROR", "/dev/stderr", 1);
	setenv("ZLOG_CHECK_FORMAT_RULE", "1", 1);

	rc = zlog_init(NULL);
	if (rc) {
		fprintf(stderr, "zlog_init fail, quit\n");
		return -1;
	}

	zt = zlog_get_category("zlog-gen-conf");
	if (!zt) {
		fprintf(stderr, "zlog_get_category fail, quit\n");
		rc = -1;
		goto main_exit;
	}

	if (argc == 0) {
		strcpy(file_name, "zlog.conf");
	} else {
		nwrite = snprintf(file_name, sizeof(file_name), "%s", *argv);
		if (nwrite < 0 || nwrite >= sizeof(file_name)) {
			ZLOG_ERROR(zt, "argv[%s] is longer than %d, quit\n", argv, sizeof(file_name));
			rc = -1;
			goto main_exit;
		}
	}

	fp = fopen(file_name, "w");
	if (!fp) {
		ZLOG_ERROR(zt, "fopen fail, errno[%d]", errno);
		rc = -1;
		goto main_exit;
	}

	switch(comment) {
	case 0:
		rc = fputs(CONF_STRING, fp);
		break;
	case 1:
		rc = fputs(CONF_STRING_CN, fp);
		break;
	case 2:
		rc = fputs(CONF_STRING_EN, fp);
		break;
	default :
		rc = 0;
		break;
	}
	if (rc == EOF) {
		ZLOG_ERROR(zt, "fputs fail, errno[%d]", errno);
		rc = -1;
		goto main_exit;
	} else {
		rc = 0;
	}

     main_exit:
	if (fp) {
		fclose(fp);
	}
	zlog_fini();
	return rc;
}
