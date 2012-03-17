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
"########\n"   \
"# 全局设置以@开头\n"  \
"# 全局设置可以不写，使用内置的默认值\n"  \
"# 语法:\n"   \
"#     @[键][n个空格或tab][值]\n"   \
"\n"   \
"### 如果 ignore_error_format_rule 是 true,\n"   \
"#    zlog_init() 将会忽略错误的format和rule.\n"   \
"#    否则zlog_init()将会严格的检查所有format和rule的语法\n"   \
"#    碰到错误的就返回-1.\n"   \
"# 默认值:\n"   \
"# @ignore_error_format_rule          false\n"  \
"\n"   \
"### zlog 在堆上为每个线程申请缓存.\n"   \
"# buf_size_min 是单个缓存的最小值.\n"   \
"# 写日志的时候, 如果日志长度大于缓存,\n"   \
"#     缓存会自动扩充到buf_size_max,\n"   \
"#     日志再长超过buf_size_max就会截断.\n"   \
"# 如果 buf_size_max 是 0, 意味着不限制缓存,\n"   \
"#     每次扩充 buf_size = 2*buf_size,\n"   \
"# 缓存大小可以加上 KB, MB 或 GB这些单位\n"   \
"# 默认值:\n"   \
"# @buf_size_min                      1024\n"  \
"# @buf_size_max                      2MB\n"  \
"\n"   \
"### rotate_lock_file 是一个posix文件锁文件\n"   \
"#     用来在多进程情况下，保证日志安全转档.\n"   \
"# zlog 会在 zlog_init()时候创建这个文件\n"   \
"# 确认你执行程序的用户有权限创建和读写这个文件.\n"   \
"# 如果有多个用户需要转档同一个日志文件\n"   \
"#     确认这个锁文件对于多个用户都可读写,\n"   \
"# 默认值:\n"   \
"# @rotate_lock_file                  /tmp/zlog.lock\n"  \
"\n"   \
"### default_format 是缺省的日志格式\n"  \
"# 内置的缺省格式会产生类似这样的输出:\n"   \
"# 2012-02-14 17:03:12 INFO [3758:test_hello.c:39] hello, zlog\n"   \
"# 可以把缺省格式设成你喜欢的样子\n" \
"# 默认值:\n"   \
"# @default_format                    \"%d(%F %T) %V [%p:%F:%L] %m%n\"\n"  \
"\n"   \
"### 用户自定义级别\n"   \
"# zlog内置的级别是:\n"   \
"# @level                             DEBUG = 20, LOG_DEBUG\n"   \
"# @level                             INFO = 40, LOG_INFO\n"   \
"# @level                             NOTICE = 60, LOG_NOTICE\n"   \
"# @level                             WARN = 80, LOG_WARNING\n"   \
"# @level                             ERROR = 100, LOG_ERR\n"   \
"# @level                             FATAL = 120, LOG_ALERT\n"   \
"# @level                             UNKNOWN = 254, LOG_ERR\n"   \
"# 语法:\n"   \
"# @level[n个空格或tab][级别的字符串] = [级别数值][syslog级别, 可选]\n"   \
"# 级别数值要在 [1,253]内, 越大代表越重要\n"   \
"# yslog级别是可选，不设的话默认是LOG_DEBUG\n"   \
"# 定义自己的级别还需要在用户的.h内写相应的宏\n"   \
"# 详见 ~/test/test_level.c ~/test/test_level.h\n"   \
"# 例如:\n"   \
"# @level                             TEST = 50\n"   \
"\n"   \
"########\n"   \
"# Format(格式)以&开头.\n"   \
"# 语法:\n"   \
"#     &[格式名][n个空格或tab]\"[格式字符串]\"\n"   \
"# 例如\n"  \
"# &simple     \"%m%n\"\n"    \
"\n"   \
"# 合法的格式和分类名需要在[a-Z][0-9][_]内\n"   \
"# 格式字符串可以是常量, 或者是转换字符(见最下面的附录)\n"   \
"\n"   \
"########\n"   \
"# Rule(规则)没有开头字符.\n"   \
"# 主语法为:\n"   \
"# [选择器][n个空格或tab][输出器]\n"   \
"\n"   \
"# *.*                     >stdout\n"   \
"# [选择器] = [分类名].[!=, 可选][级别]\n"   \
"#     [分类名] 可以是\n"   \
"#         *, 匹配所有分类\n"   \
"#         !, 匹配那些还没有规则的分类\n"   \
"#         普通分类名, 不以'_'结束, 精确匹配.\n"   \
"#             例子. aa_bb 匹配 zt = zlog_get_category(\"aa_bb\")\n"   \
"#         父分类名, 以'_'结束, 匹配父分类及子分类\n"   \
"#             例子. aa_ 匹配 zt1 = zlog_get_category(\"aa\"), zt2 = zlog_get_category(\"aa_bb\"),\n"   \
"#                             zt3 = zlog_get_category(\"aa_bb_cc\")...\n"   \
"#     [级别] 可以是zlog内置的5个级别, 或者用户自定义的级别\n" \
"#                 有下面的表达式\n" \
"#         aa.debug, soucre log action >= debug\n" \
"#         aa.!debug, source log action != debug\n" \
"#         aa.=debug, source log action == debug\n" \
"# [输出器] = [输出], [文件大小个数限制, 可选]; [format(格式)名, 可选]\n"   \
"#     [输出] 可以是\n"   \
"#         >stdout, 标准输出\n"   \
"#         >stderr, 标准错误输出\n"   \
"#         >syslog, syslog输出, 后面要跟syslog facility\n"   \
"#         \"日志文件路径\", 相对路径或绝对路径\n"   \
"#             可以内含转换字符(见下面的附录).\n"    \
"#     [文件大小个数限制]\n"  \
"#         如果 [输出] 是>syslog, 这个字段就是 syslog facility, \n"  \
"#            必须在 LOG_LOCAL[0-7] 或 LOG_USER内\n"   \
"#         如果输出是日志文件, 则是单个文件大小限制 * 文件个数限制(可选),\n"  \
"#            后面可以跟kb,Mb,G这种单位 \n"   \
"#            例子. 2048K, 1M, 10mb\n"   \
"#                  1M * 5, 10mb * 10\n"   \
"#     [format(格式名)]\n"   \
"#         不写的话使用一开始设置的缺省格式, 否则就需要是在format(格式)段定义好的格式名\n"  \
"#\n"   \
"######## 附录, 转换字符\n"   \
"# %c    源代码中的分类名                   aa_bb\n"  \
"# %d    时间日期 %d(strftime format)       12-01 17:17:42\n"   \
"#       详见man 3 strftime, zlog 增加了 %ms 毫秒 %us 微秒\n"   \
"# %E    环境变量的值 %E(HOME)              /home/test\n"   \
"# %F    源文件名, __FILE__                 test_hello.c\n"   \
"# %f    去掉路径的源文件名, __FILE__       test_hello.c\n"   \
"# %H    主机名, gethostname()              zlog-dev\n"   \
"# %L    源文件行数, __LINE__               123\n"   \
"# %m    用户消息                           hello, zlog\n"   \
"# %M    mdc, %M(key)                       value\n"   \
"# %n    换行符                             \\n\n"   \
"# %p    进程号, getpid()                   13423\n"   \
"# %V    日志等级, 大写                     INFO\n"   \
"# %v    日志等级, 小写                     info\n"   \
"# %t    线程号, pthread_self               12343\n"   \
"# %%   百分号                             %\n"   \
"# %[其他字符]   语法错误, 将会导致zlog_init()失败\n"  \
"#\n"  \
"######## 附录, 工具\n"   \
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
"# syslog level is optional, if not set, is LOG_DEBUG\n"   \
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
"# [selector] = [category].[level]\n"   \
"#     [category] should be\n"   \
"#         *, matches all category\n"   \
"#         !, matches category that has no rule matched yet\n"   \
"#         normal category, not end with '_', accurate match.\n"   \
"#             eg. aa_bb matches zt = zlog_get_category(\"aa_bb\")\n"   \
"#         super-category, end with '_', match super-category and sub-categories\n"   \
"#             eg. aa_ matches zt1 = zlog_get_category(\"aa\"), zt2 = zlog_get_category(\"aa_bb\"),\n"   \
"#                             zt3 = zlog_get_category(\"aa_bb_cc\")...\n"   \
"#     [level] should be zlog inner default levels, or user-defined levels\n" \
"#                The level expresions are\n"   \
"#         aa.debug, soucre log action >= debug\n" \
"#         aa.!debug, source log action != debug\n" \
"#         aa.=debug, source log action == debug\n" \
"# [action] = [output], [file limitation,optional]; [format name, optional]\n"   \
"#     [output] can be\n"   \
"#         >stdout, to posix stranded output\n"   \
"#         >stderr, to posix stranded error\n"   \
"#         >syslog, to syslog, must flow syslog facility\n"   \
"#         \"file path\", can be absolute file path\n"   \
"#             or relative file path with conversion pattern.\n"    \
"#     [file limitation]\n"  \
"#         if [output] is >syslog, then here is syslog facility, \n"  \
"#            must be LOG_LOCAL[0-7] or LOG_USER\n"   \
"#         else it controls file size in bytes * file count(optional),\n"   \
"#            and can be appended with kb,mb,gb \n"   \
"#            eg. 2048K, 1M, 10mb\n"   \
"#                1M * 5, 10mb * 10\n"   \
"#     [format name]\n"   \
"#         the format mentioned above, or the default format if not set\n"  \
"#\n"   \
"######## Appendix, Conversion Character\n"   \
"# %c    category name                      aa_bb\n"  \
"# %d    data & time %d(strftime format)    %d(%F %T) 2012-01 17:17:42\n"   \
"#       see man 3 strftime, zlog add %ms for millisecond and %us for microsend\n"   \
"# %E    environment variable %E(HOME)      /home/test\n"   \
"# %F    file name, __FILE__                test_hello.c\n"   \
"# %f    strip file name, __FILE__          test_hello.c\n"   \
"# %H    host name, gethostname()           zlog-dev\n"   \
"# %L    line number, __LINE__              123\n"   \
"# %m    user message                       hello, zlog\n"   \
"# %M    mdc, %M(key)                       value\n"   \
"# %n    newline                            \\n\n"   \
"# %p    process id, getpid()               13423\n"   \
"# %V    log level, capital                 INFO\n"   \
"# %v    log level, lowercase               info\n"   \
"# %t    thread id, pthread_self            12343\n"   \
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
		"Useage: zlog-gen-conf [conf file]\n"
		"If no filename is specified, use zlog.conf as default\n"
		"\t-c \tChinese comment(UTF-8)\n"
		"\t\t\tif envrionment is GBK, use\n"
		"\t\t\t$ iconv -f UTF-8 -t GBK xx.conf > yy.conf\n"
		"\t\t\t$ mv yy.conf xx.conf\n"
		"\t-e \tEnligsh comment\n"
		"\t-h,\tshow help message\n";

	while((op = getopt(argc, argv, "ceh")) > 0) {
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
