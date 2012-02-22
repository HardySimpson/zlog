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
#include <ctype.h>

#include "level.h"
#include "zc_defs.h"
#include "syslog.h"

static zlog_level_t zlog_env_level[256] = {
	[0] = {"*", sizeof("*")-1, LOG_INFO},
	[20] = {"DEBUG", sizeof("DEBUG")-1, LOG_DEBUG},
	[40] = {"INFO", sizeof("INFO")-1, LOG_INFO},
	[60] = {"NOTICE", sizeof("NOTICE")-1, LOG_NOTICE},
	[80] = {"WARN", sizeof("WARN")-1, LOG_WARNING},
	[100] = {"ERROR", sizeof("ERROR")-1, LOG_ERR},
	[120] = {"FATAL", sizeof("FATAL")-1, LOG_ALERT},
	[254] = {"UNKOWN", sizeof("UNKOWN")-1, LOG_ERR},
	[255] = {"0", sizeof("0")-1, LOG_INFO},
};

static size_t npriorities = sizeof(zlog_env_level) / sizeof(zlog_env_level[0]);

int zlog_level_atoi(char *str)
{
	int i;

	if (str == NULL || *str == '\0') {
		zc_error("str is [%s], can't find level", str);
		return -1;
	}

	for (i = 0; i < npriorities; i++) {
		if (STRICMP(str, ==, (zlog_env_level[i]).str)) {
			return i;
		}
	}

	zc_error("str[%s] can't found in level map", str);
	return -1;
}

zlog_level_t *zlog_level_get(int p)
{
	if ((p <= 0) || (p > 254)) {
		/* illegal input from zlog() */
		zc_error("p[%d] not in (0,254), set to UNKOWN", p);
		p = 254;
	}

	if (((zlog_env_level[p]).str)[0] == '\0') {
		/* empty slot */
		zc_error("p[%d] in (0,254), but not in map,"
			"see configure file define, set ot UNKOWN", p);
		p = 254;
	}
	return &(zlog_env_level[p]);
}

static int syslog_level_atoi(char *str)
{
	/* guess no unix system will choose -187
	 * as its syslog level, so it is a safe return value
	 */
	zc_assert_debug(str, -187);

	if (STRICMP(str, ==, "LOG_EMERG"))
		return LOG_EMERG;
	if (STRICMP(str, ==, "LOG_ALERT"))
		return LOG_ALERT;
	if (STRICMP(str, ==, "LOG_CRIT"))
		return LOG_CRIT;
	if (STRICMP(str, ==, "LOG_ERR"))
		return LOG_ERR;
	if (STRICMP(str, ==, "LOG_WARNING"))
		return LOG_WARNING;
	if (STRICMP(str, ==, "LOG_NOTICE"))
		return LOG_NOTICE;
	if (STRICMP(str, ==, "LOG_INFO"))
		return LOG_INFO;
	if (STRICMP(str, ==, "LOG_DEBUG"))
		return LOG_DEBUG;

	zc_error("wrong syslog level[%s]", str);
	return -187;
}

int zlog_level_set(char *str, int l, char *sl)
{
	zlog_level_t a_level;
	int i;

	zc_assert_debug(str, -1);
	zc_assert_debug(sl, -1);

	/* check level and str */
	if ((l <= 0) || (l > 254)) {
		zc_error("l[%d] not in (0,254), wrong", l);
		return -1;
	}

	if (*str == '\0') {
		zc_error("str[0] = 0");
		return -1;
	}

	memset(&a_level, 0x00, sizeof(a_level));

	/* fill syslog level */
	if (*sl == '\0') {
		a_level.syslog_level = LOG_DEBUG;
	} else {
		a_level.syslog_level = syslog_level_atoi(sl);
		if (a_level.syslog_level == -187) {
			zc_error("syslog_level_atoi fail");
			return -1;
		}
	}

	/* strncpy and toupper(str)  */
	for (i = 0; (i < sizeof(a_level.str) - 1) && str[i] != '\0'; i++) {
		(a_level.str)[i] = toupper(str[i]);
	}

	if (str[i] != '\0') {
		/* overflow */
		zc_error("not enough space for str, str[%s] > %d", str, i);
		return -1;
	} else {
		(a_level.str)[i] = '\0';
	}

	a_level.str_len = i;

	/* all success, then copy, keep consistency */
	memcpy(&(zlog_env_level[l]), &a_level, sizeof(a_level));

	return 0;
}

