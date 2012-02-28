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
#include <errno.h>

#include "zc_defs.h"
#include "level.h"
#include "syslog.h"

static zc_arraylist_t *zlog_env_levels;

static int zlog_levels_set_default(void)
{
	return zlog_level_set("* = 0, LOG_INFO")
	|| zlog_level_set("DEBUG = 20, LOG_DEBUG")
	|| zlog_level_set("INFO = 40, LOG_INFO")
	|| zlog_level_set("NOTICE = 60, LOG_NOTICE")
	|| zlog_level_set("WARN = 80, LOG_WARNING")
	|| zlog_level_set("ERROR = 100, LOG_ERR")
	|| zlog_level_set("FATAL = 120, LOG_ALERT")
	|| zlog_level_set("UNKNOWN = 254, LOG_ERR")
	|| zlog_level_set("! = 255, LOG_INFO");
}

void zlog_levels_fini(void)
{
	if (zlog_env_levels) {
		zc_arraylist_del(zlog_env_levels);
	}
	zlog_env_levels = NULL;
	return;
}

int zlog_levels_init(void)
{
	int rc;

	zlog_env_levels = zc_arraylist_new(free);
	if (!zlog_env_levels) {
		zc_error("zc_arraylist_new fail");
		return -1;
	}

	rc = zlog_levels_set_default();
	if (rc) {
		zc_error("zlog_level_set_default fail");
		rc = -1;
		goto zlog_level_init_exit;
	}

      zlog_level_init_exit:
	if (rc) {
		zlog_levels_fini();
		return -1;
	} else {
		return 0;
	}
}

int zlog_levels_reset(void)
{
	zlog_levels_fini();
	return zlog_levels_init();
}

int zlog_level_atoi(char *str)
{
	int i;
	zlog_level_t *a_level;

	if (str == NULL || *str == '\0') {
		zc_error("str is [%s], can't find level", str);
		return -1;
	}

	zc_arraylist_foreach(zlog_env_levels, i, a_level) {
		if (a_level && STRICMP(str, ==, a_level->str_capital)) {
			return i;
		}
	}

	zc_error("str[%s] can't found in level map", str);
	return -1;
}

zlog_level_t *zlog_level_get(int p)
{
	zlog_level_t *a_level;

	if ((p <= 0) || (p > 254)) {
		/* illegal input from zlog() */
		zc_error("p[%d] not in (0,254), set to UNKOWN", p);
		p = 254;
	}

	a_level = zc_arraylist_get(zlog_env_levels, p);
	if (!a_level) {
		/* empty slot */
		zc_error("p[%d] in (0,254), but not in map,"
			"see configure file define, set ot UNKOWN", p);
		a_level = zc_arraylist_get(zlog_env_levels, 254);
	}

	return a_level;
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

int zlog_level_set(char *line)
{
	int rc;
	zlog_level_t *a_level;
	int i;
	int nread;
	char str[MAXLEN_CFG_LINE + 1];
	int l;
	char sl[MAXLEN_CFG_LINE + 1];

	zc_assert_debug(line, -1);

	nread = sscanf(line, " %[^= ] = %d ,%s",
		str, &l, sl);
	if (nread < 2) {
		zc_error("level[%s] syntax wrong", line);
		return -1;
	}

	/* check level and str */
	if ((l < 0) || (l > 255)) {
		zc_error("l[%d] not in [0,255], wrong", l);
		return -1;
	}

	if (*str == '\0') {
		zc_error("str[0] = 0");
		return -1;
	}

	a_level = calloc(1, sizeof(*a_level));
	if (!a_level) {
		zc_error("calloc fail, errno[%d]", errno);
		return -1;
	}

	/* fill syslog level */
	if (*sl == '\0') {
		a_level->syslog_level = LOG_DEBUG;
	} else {
		a_level->syslog_level = syslog_level_atoi(sl);
		if (a_level->syslog_level == -187) {
			zc_error("syslog_level_atoi fail");
			rc = -1;
			goto zlog_level_set_exit;
		}
	}

	/* strncpy and toupper(str)  */
	for (i = 0; (i < sizeof(a_level->str_capital) - 1) && str[i] != '\0'; i++) {
		(a_level->str_capital)[i] = toupper(str[i]);
		(a_level->str_lowercase)[i] = tolower(str[i]);
	}

	if (str[i] != '\0') {
		/* overflow */
		zc_error("not enough space for str, str[%s] > %d", str, i);
		rc = -1;
		goto zlog_level_set_exit;
	} else {
		(a_level->str_capital)[i] = '\0';
		(a_level->str_lowercase)[i] = '\0';
	}

	a_level->str_len = i;

	/* all success, then copy, keep consistency */
	rc = zc_arraylist_set(zlog_env_levels, l, a_level);
	if (rc) {
		zc_error("zc_arraylist_set fail");
		rc = -1;
		goto zlog_level_set_exit;
	}

      zlog_level_set_exit:
	if (rc) {
		zc_error("line[%s]", line);
		free(a_level);
		return -1;
	} else {
		return 0;
	}
}

void zlog_levels_profile(void)
{
	int i;
	zlog_level_t *a_level;

	zc_arraylist_foreach(zlog_env_levels, i, a_level) {
		if (a_level) {
			zc_error("level:%s = %d, %d",
				a_level->str_capital, 
				i, a_level->syslog_level);
		}
	}

	return;
}
