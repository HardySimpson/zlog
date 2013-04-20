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

#include "fmacros.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "zc_defs.h"

        #include "event.h"
        #include "mdc.h"
      #include "thread.h"
    #include "format.h"
    #include "rotater.h"
  #include "conf.h"
#include "rule.h"

  #include "level.h"
#include "level_list.h"

/*******************************************************************************/
#ifdef BUFSIZ /* get the size from stdio.h */
#  define ZLOG_CONF_DEFAULT_BUF_SIZE BUFSIZ
#else
#  define ZLOG_CONF_DEFAULT_BUF_SIZE 8192
#endif
#define ZLOG_CONF_DEFAULT_FORMAT_LINE "defalut = \"%d %V [%p:%F:%L] %m%n\""
#define ZLOG_CONF_BACKUP_ROTATE_LOCK_FILE "/tmp/zlog.lock"
#define ZLOG_CONF_DEFAULT_FILE_PERMS 0600
#define ZLOG_CONF_DEFAULT_RELOAD_CONF 0
#define ZLOG_CONF_DEFAULT_FSYNC_PERIOD 0
#define ZLOG_CONF_DEFAULT_RULE "*.*        >stdout"
/*******************************************************************************/

void zlog_conf_profile(zlog_conf_t * a_conf, int flag)
{
	int i;
	zlog_rule_t *a_rule;
	zlog_format_t *a_format;

	zc_assert(a_conf,);
	zc_profile(flag, "-conf[%p]-", a_conf);
	zc_profile(flag, "--global--");
	zc_profile(flag, "---file[%s],mtime[%d]---", a_conf->file, a_conf->mtime);
	zc_profile(flag, "---strict init[%d]---", a_conf->strict_init);
	zc_profile(flag, "---buffer min[%ld]---", (long) a_conf->buf_size);
	if (a_conf->default_format) {
		zc_profile(flag, "---default_format---");
		zlog_format_profile(a_conf->default_format, flag);
	}
	zc_profile(flag, "---file perms[0%o]---", a_conf->file_perms);
	zc_profile(flag, "---reload conf[%d]---", (int) a_conf->reload_conf);
	zc_profile(flag, "---fsync period[%ld]---", (long) a_conf->fsync_write);

	zc_profile(flag, "---rotate lock file[%s]---", a_conf->rotate_lock_file);
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
	if (a_conf->file) zc_sdsfree(a_conf->file);
	if (a_conf->rotate_lock_file) zc_sdsfree(a_conf->rotate_lock_file);
	if (a_conf->default_format_line) zc_sdsfree(a_conf->default_format_line);
	if (a_conf->rotater) zlog_rotater_del(a_conf->rotater);
	if (a_conf->default_format) zlog_format_del(a_conf->default_format);
	if (a_conf->levels) zlog_level_list_del(a_conf->levels);
	if (a_conf->formats) zc_arraylist_del(a_conf->formats);
	if (a_conf->rules) zc_arraylist_del(a_conf->rules);
	free(a_conf);
	zc_debug("zlog_conf_del[%p]");
	return;
}

static int zlog_conf_build_without_file(zlog_conf_t * a_conf);
static int zlog_conf_build_with_file(zlog_conf_t * a_conf);

zlog_conf_t *zlog_conf_new(const char *confpath)
{
	int rc;
	zlog_conf_t *a_conf = NULL;

	a_conf = calloc(1, sizeof(zlog_conf_t));
	if (!a_conf) { zc_error("calloc fail, errno[%d]", errno); return NULL; }

	/* global section default*/
	a_conf->strict_init = 1;
	a_conf->buf_size = ZLOG_CONF_DEFAULT_BUF_SIZE;
	a_conf->default_format_line = zc_sdsnew(ZLOG_CONF_DEFAULT_FORMAT_LINE);
	if (!a_conf->default_format_line) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }
	a_conf->rotate_lock_file = zc_sdsnew(ZLOG_CONF_BACKUP_ROTATE_LOCK_FILE);
	if (!a_conf->rotate_lock_file) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }
	a_conf->file_perms = ZLOG_CONF_DEFAULT_FILE_PERMS;
	a_conf->reload_conf = ZLOG_CONF_DEFAULT_RELOAD_CONF;
	a_conf->fsync_write = ZLOG_CONF_DEFAULT_FSYNC_PERIOD;

	a_conf->levels = zlog_level_list_new();
	if (!a_conf->levels) { zc_error("zlog_level_list_new fail"); goto err; } 
	a_conf->formats = zc_arraylist_new((zc_arraylist_del_fn) zlog_format_del);
	if (!a_conf->formats) { zc_error("zc_arraylist_new fail"); goto err; }
	a_conf->rules = zc_arraylist_new((zc_arraylist_del_fn) zlog_rule_del);
	if (!a_conf->rules) { zc_error("init rule_list fail"); goto err; }
	if (confpath && confpath[0] != '\0') {
		a_conf->file = zc_sdsnew(confpath);
		if (!a_conf->file) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }
	} else if (getenv("ZLOG_CONF_PATH") != NULL) {
		a_conf->file = zc_sdsnew(getenv("ZLOG_CONF_PATH"));
		if (!a_conf->file) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }
	} /* else a_conf->file == NULL */

	if (a_conf->file) {
		/* configure file as default lock file, overwrite backup */
		zc_sdscpy(a_conf->rotate_lock_file , a_conf->file);
		if (!a_conf->rotate_lock_file) { zc_error("zc_sdsdup fail, errno[%d]", errno); goto err; }

		rc = zlog_conf_build_with_file(a_conf);
		if (rc) { zc_error("zlog_conf_build_with_file fail"); goto err; }
	} else {
		rc = zlog_conf_build_without_file(a_conf);
		if (rc) { zc_error("zlog_conf_build_without_file fail"); goto err; }
	}

	zlog_conf_profile(a_conf, ZC_DEBUG);
	return a_conf;
err:
	zlog_conf_del(a_conf);
	return NULL;
}
/*******************************************************************************/
static int zlog_conf_build_without_file(zlog_conf_t * a_conf)
{
	zlog_rule_t *default_rule;

	a_conf->rotater = zlog_rotater_new(a_conf->rotate_lock_file);
	if (!a_conf->rotater) { zc_error("zlog_rotater_new fail"); return -1; }

	a_conf->default_format = zlog_format_new(a_conf->default_format_line, &(a_conf->time_spec_count));
	if (!a_conf->default_format) { zc_error("zlog_format_new fail"); return -1; }

	default_rule = zlog_rule_new(ZLOG_CONF_DEFAULT_RULE, a_conf);
	if (!default_rule) { zc_error("zlog_rule_new fail"); return -1; }

	if (zc_arraylist_add(a_conf->rules, default_rule)) {
		zlog_rule_del(default_rule);
		zc_error("zc_arraylist_add fail");
		return -1;
	}

	return 0;
}
/*******************************************************************************/
static int zlog_conf_parse_line(zlog_conf_t * a_conf, zc_sds line, int *section);

static int zlog_conf_build_with_file(zlog_conf_t * a_conf)
{
	int rc = 0;
	struct stat a_stat;
	FILE *fp = NULL;

	char line_buf[1024 + 1];
	char *line_start;
	char *p = NULL;
	int in_quotation = 0;
	zc_sds config = zc_sdsempty();

	zc_sds *lines = NULL;
	int line_count, i;
	int section = 0; /* [global:1] [levels:2] [formats:3] [rules:4] */

	rc = lstat(a_conf->file, &a_stat);
	if (rc) { zc_error("lstat conf file[%s] fail, errno[%d]", a_conf->file, errno); return -1; }
	a_conf->mtime = a_stat.st_mtime;

	fp = fopen(a_conf->file, "r");
	if (!fp) { zc_error("open conf file[%s] fail, errno[%d]", a_conf->file, errno); return -1; }

	/* Now process the file.
	 */
	while(fgets(line_buf, sizeof(line_buf), fp) != NULL) {

		/* check for end-of-section, comments, strip off trailing
		 * spaces and newline character.
		 */
		for (p = line_buf; (*p && isspace((int)*p)); p++);
		if (*p == '\0' || *p == '#') continue;
		line_start = p;

		/* clean the tail comments start from # and not in quotation */
		in_quotation = 0;
		for (p = line_start; *p != '\0'; p++) {
			if (*p == '"') {
				in_quotation ^= 1;
				continue;
			}

			if (*p == '#' && !in_quotation) {
				*p = '\n';
				*(p+1) = '\0';
				break;
			}
		}

		/* if the end of line is '\',
		 * the next line will be connected to the line now
		 */
		for (/* p = line_end */; p > line_start; p--) {
			if (isspace((int)*p)) continue;
			if (*p == '\\') {
				*p = '\0';
				break;
			}
		}

		config = zc_sdscat(config, line_start);
		if (!config) { zc_error("zc_sdscat fail, errno[%d]", errno); goto err; }
	}

	lines = zc_sdssplitlen(config,zc_sdslen(config),"\n",1,&line_count);
	if (!lines) { zc_error("zc_sdssplitlen fail, errno[%d]", errno); goto err; }

	for (i = 0; i < line_count; i++) {
		rc = zlog_conf_parse_line(a_conf, lines[i], &section);
		if (rc && a_conf->strict_init) {
			zc_error("parse configure file[%s]line_no[%ld] fail", a_conf->file, i);
			zc_error("line[%s]", lines[i]);
			goto err;
		} else if (rc && !a_conf->strict_init) {
			zc_warn("parse configure file[%s]line_no[%ld] fail", a_conf->file, i);
			zc_warn("line[%s]", lines[i]);
			zc_warn("as strict init is set to false, ignore and go on");
			rc = 0;
			continue;
		}
	}

	if (lines) zc_sdsfreesplitres(lines, line_count);
	if (fp) fclose(fp);
	if (config) zc_sdsfree(config);
	return 0;
err:
	if (lines) zc_sdsfreesplitres(lines, line_count);
	if (fp) fclose(fp);
	if (config) zc_sdsfree(config);
	return -1;
}

/* section [global:1] [levels:2] [formats:3] [rules:4] */
static int zlog_conf_parse_line(zlog_conf_t * a_conf, zc_sds line, int *section)
{
	int argc;
	zc_sds *argv = NULL;
	zlog_format_t *a_format = NULL;
	zlog_rule_t *a_rule = NULL;

	line = zc_sdstrim(line, " \t\r\n");
	argv = zc_sdssplitargs(line, "=", &argc);
	if (!argv) { zc_error("Unbalanced quotes in configuration line"); goto err; }

	/* process section tag */
	if (line[0] == '[') {
		int last_section = *section;

		line = zc_sdstrim(line, "[] \t\r\n");
		if (STRICMP(line, ==, "global")) {
			*section = 1;
		} else if (STRICMP(line, ==, "levels")) {
			*section = 2;
		} else if (STRICMP(line, ==, "formats")) {
			*section = 3;
		} else if (STRICMP(line, ==, "rules")) {
			*section = 4;
		} else {
			zc_error("wrong section name[%s]", line);
			goto err;
		}

		/* check the sequence of section, must increase */
		if (last_section >= *section) {
			zc_error("wrong sequence of section, must follow global->levels->formats->rules");
			goto err;
		}

		if (*section == 4) {
			/* now build rotater and default_format when setcion 4 [rules] starts
			 * from the unchanging global setting,
			 * for zlog_rule_new() */
			a_conf->rotater = zlog_rotater_new(a_conf->rotate_lock_file);	
			if (!a_conf->rotater) { zc_error("zlog_rotater_new fail"); goto err; }

			a_conf->default_format = zlog_format_new(a_conf->default_format_line, &(a_conf->time_spec_count));
			if (!a_conf->default_format) { zc_error("zlog_format_new fail"); goto err; }
		}
		goto exit;
	} 

	/* process detail */
	switch (*section) {
	case 1:
		if (STRICMP(argv[0], ==, "strict") && STRICMP(argv[1], ==, "init")) {
			/* environment variable ZLOG_STRICT_INIT > conf setting */
			if (STRICMP(argv[2], ==, "false") && !getenv("ZLOG_STRICT_INIT")) {
				a_conf->strict_init = 0;
			} else {
				a_conf->strict_init = 1;
			}
		} else if (STRICMP(argv[0], ==, "buffer") && STRICMP(argv[1], ==, "size")) {
			a_conf->buf_size = zc_parse_byte_size(argv[2]);
		} else if (STRICMP(argv[0], ==, "file") && STRICMP(argv[1], ==, "perms")) {
			sscanf(argv[2], "%o", &(a_conf->file_perms));
		} else if (STRICMP(argv[0], ==, "rotate") && STRICMP(argv[1], ==, "lock") && STRICMP(argv[2], ==, "file")) {
			/* may overwrite the inner default value, or last value */
			if (STRICMP(argv[3], ==, "self")) {
				zc_sdscpy(a_conf->rotate_lock_file, a_conf->file);
			} else {
				zc_sdscpy(a_conf->rotate_lock_file, argv[4]);
			}
		} else if (STRICMP(argv[0], ==, "default") && STRICMP(argv[1], ==, "format")) {
			/* so the input now is [default = "xxyy"], fit format's style */
			zc_sdsclear(a_conf->default_format_line);
			zc_sdscatprintf(a_conf->default_format_line, "default = %s", argv[2]);
		} else if (STRICMP(argv[0], ==, "reload") && STRICMP(argv[1], ==, "conf")) {
			a_conf->reload_conf = zc_parse_byte_size(argv[2]);
		} else if (STRICMP(argv[0], ==, "fsync") && STRICMP(argv[1], ==, "write")) {
			a_conf->fsync_write = zc_parse_byte_size(argv[2]);
		} else {
			zc_error("[%s] is not any one of global options", line);
			goto err;
		}
		break;
	case 2:
		if (zlog_level_list_set(a_conf->levels, line)) {
			zc_error("zlog_level_list_set fail");
			goto err;
		}
		break;
	case 3:
		a_format = zlog_format_new(line , &(a_conf->time_spec_count));
		if (!a_format) { zc_error("zlog_format_new fail [%s]", line); goto err; }
		if (zc_arraylist_add(a_conf->formats, a_format)) {
			zlog_format_del(a_format);
			zc_error("zc_arraylist_add fail");
			goto err;
		}
		break;
	case 4:
		a_rule = zlog_rule_new(line, a_conf);
		if (!a_rule) { zc_error("zlog_rule_new fail [%s]", line); goto err; }
		if (zc_arraylist_add(a_conf->rules, a_rule)) {
			zlog_rule_del(a_rule);
			zc_error("zc_arraylist_add fail");
			goto err;
		}
		break;
	default:
		zc_error("not in any section");
		goto err;
		break;
	}

exit:
	if (argv) zc_sdsfreesplitres(argv, argc);
	return 0;
err:
	if (argv) zc_sdsfreesplitres(argv, argc);
	return -1;
}
/*******************************************************************************/
