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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "conf.h"
#include "rule.h"
#include "format.h"
#include "level_list.h"
#include "rotater.h"
#include "zc_defs.h"

/*******************************************************************************/
#define ZLOG_CONF_DEFAULT_FORMAT "default   \"%d(%F %T) %V [%p:%F:%L] %m%n\""
#define ZLOG_CONF_DEFAULT_RULE "*.*        >stdout"
#define ZLOG_CONF_DEFAULT_BUF_SIZE_MIN 1024
#define ZLOG_CONF_DEFAULT_BUF_SIZE_MAX (2 * 1024 * 1024)
#define ZLOG_CONF_DEFAULT_ROTATE_LOCK_FILE "/tmp/zlog.lock"
/*******************************************************************************/

struct zlog_conf_s {
	char file[MAXLEN_PATH + 1];
	char mtime[20 + 1];

	int strict_init;
	size_t buf_size_min;
	size_t buf_size_max;

	char rotate_lock_file[MAXLEN_CFG_LINE + 1];
	zlog_rotater_t *rotater;

	zc_arraylist_t *levels;

	char default_format_line[MAXLEN_CFG_LINE + 1];
	zlog_format_t *default_format;

	zc_arraylist_t *formats;

	zc_arraylist_t *rules;
};

void zlog_conf_profile(zlog_conf_t * a_conf, int flag)
{
	int i;
	zlog_rule_t *a_rule;
	zlog_format_t *a_format;

	zc_assert(a_conf,);
	zc_profile(flag, "-conf[%p]-", a_conf);
	zc_profile(flag, "--global--");
	zc_profile(flag, "---file[%s],mtime[%s]---", a_conf->file, a_conf->mtime);
	zc_profile(flag, "---strict init[%d]---", a_conf->strict_init);
	zc_profile(flag, "---buffer min[%ld]---", a_conf->buf_size_min);
	zc_profile(flag, "---buffer max[%ld]---", a_conf->buf_size_max);
	zc_profile(flag, "---rotate lock file[%s]---", a_conf->rotate_lock_file);
	if (a_conf->default_format) {
		zc_profile(flag, "---default_format---");
		zlog_format_profile(a_conf->default_format, flag);
	}

	if (a_conf->rotater) zlog_rotater_profile(a_conf->rotater, flag);

	if (a_conf->levels) zlog_level_list_profile(a_conf->levels, flag);

	if (a_conf->formats) {
		zc_profile(flag, "--format list[%p]--", a_conf->formats);
		zc_arraylist_foreach(a_conf->formats, i, a_format) {
			zlog_format_profile(a_format, flag);
		}
	}

	if (a_conf->rules) {
		zc_profile(flag, "--rule_list[%p]--", a_conf->rules);
		zc_arraylist_foreach(a_conf->rules, i, a_rule) {
			zlog_rule_profile(a_rule, flag);
		}
	}

	return;
}
/*******************************************************************************/
void zlog_conf_del(zlog_conf_t * a_conf)
{
	zc_assert(a_conf,);
	if (a_conf->rotater) zlog_rotater_del(a_conf->rotater);
	if (a_conf->levels) zlog_level_list_del(a_conf->levels);
	if (a_conf->default_format) zlog_format_del(a_conf->default_format);
	if (a_conf->formats) zc_arraylist_del(a_conf->formats);
	if (a_conf->rules) zc_arraylist_del(a_conf->rules);
	free(a_conf);
	zc_debug("zlog_conf_del[%p]");
	return;
}

static int zlog_conf_build_default(zlog_conf_t * a_conf);
static int zlog_conf_read_config(zlog_conf_t * a_conf);

zlog_conf_t *zlog_conf_new(char *conf_file)
{
	int rc = 0;
	int nwrite = 0;
	zlog_conf_t *a_conf;

	a_conf = calloc(1, sizeof(zlog_conf_t));
	if (!a_conf) {
		zc_error("calloc fail, errno[%d]", errno);
		return NULL;
	}

	if (conf_file) {
		nwrite = snprintf(a_conf->file, sizeof(a_conf->file), "%s", conf_file);
	} else if (getenv("ZLOG_CONF_PATH") != NULL) {
		nwrite = snprintf(a_conf->file, sizeof(a_conf->file), "%s", getenv("ZLOG_CONF_PATH"));
	} else {
		memset(a_conf->file, 0x00, sizeof(a_conf->file));
	}
	if (nwrite < 0 || nwrite >= sizeof(a_conf->file)) {
		zc_error("not enough space for path name, nwrite=[%d]", nwrite);
		rc = -1;
		goto zlog_conf_new_exit;
	}

	rc = zlog_conf_build_default(a_conf);
	if (rc) {
		rc = -1;
		zc_error("zlog_conf_build fail");
		goto zlog_conf_new_exit;
	}

	rc = zlog_conf_read_config(a_conf);
	if (rc) {
		rc = -1;
		zc_error("zlog_conf_read_config fail");
		goto zlog_conf_new_exit;
	}

      zlog_conf_new_exit:
	if (rc) {
		zlog_conf_del(a_conf);
		return NULL;
	} else {
		zlog_conf_profile(a_conf, ZC_DEBUG);
		return a_conf;
	}
}
/*******************************************************************************/
static int zlog_conf_build_default(zlog_conf_t * a_conf)
{
	int rc = 0;
	zlog_rule_t *default_rule;

	/* set default configuration start */
	a_conf->strict_init = 1;
	a_conf->buf_size_min = ZLOG_CONF_DEFAULT_BUF_SIZE_MIN;
	a_conf->buf_size_max = ZLOG_CONF_DEFAULT_BUF_SIZE_MAX;
	strcpy(a_conf->rotate_lock_file, ZLOG_CONF_DEFAULT_ROTATE_LOCK_FILE);
	strcpy(a_conf->default_format_line, ZLOG_CONF_DEFAULT_FORMAT);

	a_conf->levels = zlog_level_list_new();
	if (!a_conf->levels) {
		zc_error("zlog_level_list_new fail");
		return -1;
	}

	a_conf->formats = zc_arraylist_new((zc_arraylist_del_fn) zlog_format_del);
	if (!a_conf->formats) {
		zc_error("zc_arraylist_new fail");
		return -1;
	}

	a_conf->rules = zc_arraylist_new((zc_arraylist_del_fn) zlog_rule_del);
	if (!a_conf->rules) {
		zc_error("init rule_list fail");
		return -1;
	}

	a_conf->default_format = zlog_format_new(a_conf->default_format_line, a_conf->levels);
	if (!a_conf->default_format) {
		zc_error("zlog_format_new fail");
		return -1;
	}

	a_conf->rotater = zlog_rotater_new(a_conf->rotate_lock_file);
	if (!a_conf->rotater) {
		zc_error("zlog_rotater_new fail");
		return -1;
	}

	/* with config file */
	if (a_conf->file[0] != '\0') {
		return 0;
	}

	/* without config file */
	default_rule = zlog_rule_new(
			ZLOG_CONF_DEFAULT_RULE,
			a_conf->rotater,
			a_conf->levels,
			a_conf->default_format,
			a_conf->formats);
	if (!default_rule) {
		zc_error("zlog_rule_new fail");
		return -1;
	}

	/* add default rule */
	rc = zc_arraylist_add(a_conf->rules, default_rule);
	if (rc) {
		zc_error("zc_arraylist_add fail");
		zlog_rule_del(default_rule);
		return -1;
	}

	return 0;
}
/*******************************************************************************/
static int zlog_conf_parse_line(zlog_conf_t * a_conf, char *line, int *section);

static int zlog_conf_read_config(zlog_conf_t * a_conf)
{
	int rc = 0;
	struct stat a_stat;
	struct tm local_time;
	FILE *fp = NULL;

	char line[MAXLEN_CFG_LINE + 1];
	size_t line_len;
	char *pline = NULL;
	char *p = NULL;
	int line_no = 0;
	int i = 0;

	int section = 0;
	/* [global:1] [levels:2] [formats:3] [rules:4] */

	/* without config file, do nothing */
	if (a_conf->file[0] == '\0') {
		return 0;
	}

	rc = lstat(a_conf->file, &a_stat);
	if (rc) {
		zc_error("lstat conf file[%s] fail, errno[%d]", a_conf->file,
			 errno);
		return -1;
	}
	localtime_r(&(a_stat.st_mtime), &local_time);
	strftime(a_conf->mtime, sizeof(a_conf->mtime), "%F %T", &local_time);

	if ((fp = fopen(a_conf->file, "r")) == NULL) {
		zc_error("open configure file[%s] fail", a_conf->file);
		return -1;
	}

	/* Now process the file.
	 */
	pline = line;
	memset(&line, 0x00, sizeof(line));
	while (fgets((char *)pline, sizeof(line) - (pline - line), fp) != NULL) {
		++line_no;
		line_len = strlen(pline);
		if (pline[line_len - 1] == '\n') {
			pline[line_len - 1] = '\0';
		}

		/* check for end-of-section, comments, strip off trailing
		 * spaces and newline character.
		 */
		p = pline;
		while (*p && isspace((int)*p))
			++p;
		if (*p == '\0' || *p == '#')
			continue;

		for (i = 0; p[i] != '\0'; ++i) {
			pline[i] = p[i];
		}
		pline[i] = '\0';

		for (p = pline + strlen(pline) - 1; isspace((int)*p); --p)
			/*EMPTY*/;

		if (*p == '\\') {
			if ((p - line) > MAXLEN_CFG_LINE - 30) {
				/* Oops the buffer is full - what now? */
				pline = line;
			} else {
				for (p--; isspace((int)*p); --p)
					/*EMPTY*/;
				p++;
				*p = 0;
				pline = p;
				continue;
			}
		} else
			pline = line;

		*++p = '\0';

		/* we now have the complete line,
		 * and are positioned at the first non-whitespace
		 * character. So let's process it
		 */
		rc = zlog_conf_parse_line(a_conf, line, &section);
		if (rc) {
			zc_error("parse configure file[%s]line:(%ld)[%s] fail",
				a_conf->file, line_no, line);
			goto zlog_conf_read_config_exit;
		}
	}

      zlog_conf_read_config_exit:

	fclose(fp);
	return rc;
}

/* section [global:1] [levels:2] [formats:3] [rules:4] */
static int zlog_conf_parse_line(zlog_conf_t * a_conf, char *line, int *section)
{
	int rc = 0;
	int nscan = 0;
	char name[MAXLEN_CFG_LINE + 1];
	char word_1[MAXLEN_CFG_LINE + 1];
	char word_2[MAXLEN_CFG_LINE + 1];
	char word_3[MAXLEN_CFG_LINE + 1];
	char value[MAXLEN_CFG_LINE + 1];
	zlog_format_t *a_format = NULL;
	zlog_rule_t *a_rule = NULL;

	if (strlen(line) > MAXLEN_CFG_LINE) {
		zc_error ("line_len[%ld] > MAXLEN_CFG_LINE[%ld], may cause overflow",
		     strlen(line), MAXLEN_CFG_LINE);
		return -1;
	}

	/* get and set outer section flag, so it is a closure? haha */
	if (line[0] == '[') {
		nscan = sscanf(line, "[ %s ]", name);
		if (STRCMP(name, ==, "global")) {
			*section = 1;
		} else if (STRCMP(name, ==, "levels")) {
			*section = 2;
		} else if (STRCMP(name, ==, "formats")) {
			*section = 3;
		} else if (STRCMP(name, ==, "rules")) {
			*section = 3;
		} else {
			zc_error("wrong section name[%s]", line);
			return -1;
		}
		return 0;
	}

	/* set detail */
	switch (*section) {
	case 1:
		memset(name, 0x00, sizeof(name));
		memset(value, 0x00, sizeof(value));
		nscan = sscanf(line, " %[^= ] = %s ", name, value);
		if (nscan != 2) {
			zc_error("sscanf [%s] fail, name or value is null", line);
			return -1;
		}

		memset(word_1, 0x00, sizeof(word_1));
		memset(word_2, 0x00, sizeof(word_2));
		memset(word_3, 0x00, sizeof(word_3));
		nscan = sscanf(name, "%s%s%s", word_1, word_2, word_3);

		if (STRCMP(word_1, ==, "strict") && STRCMP(word_2, ==, "init")) {
			/* if environment variable ZLOG_STRICT_INIT is set
			 * then always make it strict 
			 */
			if (STRICMP(value, ==, "false") && !get_env("ZLOG_STRICT_INIT")) {
				a_conf->strict_init = 0;
			} else {
				a_conf->strict_init = 1;
			}
		} else if (STRCMP(word_1, ==, "buffer") && STRCMP(word_2, ==, "min")) {
			a_conf->buf_size_min = zc_parse_byte_size(value);
		} else if (STRCMP(word_1, ==, "buffer") && STRCMP(word_2, ==, "max")) {
			a_conf->buf_size_max = zc_parse_byte_size(value);
		} else if (STRCMP(word_1, ==, "rotate") &&
				STRCMP(word_2, ==, "lock") && STRCMP(word_3, ==, "file")) {
			/* close the default or older rotater */
			if (a_conf->rotater) zlog_rotater_del(a_conf->rotater);
			strcpy(a_conf->rotate_lock_file, value);
			a_conf->rotater = zlog_rotater_new(a_conf->rotate_lock_file);	
			if (!a_conf->rotater) {
				zc_error("zlog_rotater_new fail");
				return -1;
			}
		} else if (STRCMP(word_1, ==, "default") && STRCMP(word_2, ==, "format")) {
			/* close the default or older default_format */
			if (a_conf->default_format) zlog_format_del(a_conf->default_format);
			a_conf->default_format = zlog_format_new(line, a_conf->levels);
			if (!a_conf->default_format) {
				zc_error("zlog_format_new fail");
				return -1;
			}
		} else {
			zc_error("name[%s] is not any one of global options", name);
			if (a_conf->strict_init) return -1;
		}
		break;
	case 2:
		rc = zlog_level_list_set(a_conf->levels, line);
		if (rc) {
			zc_error("zlog_level_list_set fail");
			if (a_conf->strict_init) return -1;
		}
		break;
	case 3:
		a_format = zlog_format_new(line, a_conf->levels);
		if (!a_format) {
			zc_error("zlog_format_new fail [%s]", line);
			if (a_conf->strict_init) return -1;
			else break;
		}
		rc = zc_arraylist_add(a_conf->formats, a_format);
		if (rc) {
			zlog_format_del(a_format);
			zc_error("zc_arraylist_add fail");
			return -1;
		}
		break;
	case 4:
		a_rule = zlog_rule_new(line,
			a_conf->rotater,
			a_conf->levels,
			a_conf->default_format,
			a_conf->formats);
		if (!a_rule) {
			zc_error("zlog_rule_new fail [%s]", line);
			if (a_conf->strict_init) return -1;
			else break;
		}
		rc = zc_arraylist_add(a_conf->rules, a_rule);
		if (rc) {
			zlog_rule_del(a_rule);
			zc_error("zc_arraylist_add fail");
			return -1;
		}
		break;
	default:
		zc_error("not in any section");
		return -1;
		break;
	}

	return 0;
}
