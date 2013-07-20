/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
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
    #include "deepness.h"
  #include "conf.h"
#include "rule.h"

  #include "level.h"
#include "level_list.h"


/*******************************************************************************/

#define ZLOG_CONF_BACKUP_ROTATE_LOCK_FILE	"/tmp/zlog.lock"
#define ZLOG_CONF_DEFAULT_FORMAT		"* = \"%d %V [%p:%F:%L] %m%n\""
#define ZLOG_CONF_DEFAULT_DEEPNESS		"* = 600,8192,6144,0,-1"
#define ZLOG_CONF_DEFAULT_RULE			"*.*        >stdout"

/*******************************************************************************/

void zlog_conf_profile(zlog_conf_t * a_conf, int flag)
{
	int i;
	zlog_rule_t *a_rule;
	zlog_deepness_t *a_deepness;
	zlog_format_t *a_format;

	zc_assert(a_conf,);
	zc_profile(flag, "-conf[%p]-", a_conf);
	zc_profile(flag, "--global--");

	zc_profile(flag, "---file[%s],mtime[%ld]---", a_conf->file, a_conf->mtime);
	zc_profile(flag, "---strict init[%d]---", a_conf->strict_init);
	zc_profile(flag, "---auto reload [%ld]---", a_conf->auto_reload);

	zc_profile(flag, "---rotate lock file[%s]---", a_conf->rotate_lock_file);
	zc_profile(flag, "---default deepness[%s]---", a_conf->default_deepness_str);
	zc_profile(flag, "---default format[%s]---", a_conf->default_format_str);

	if (a_conf->rotater) zlog_rotater_profile(a_conf->rotater, flag);


	if (a_conf->deepness) {
		zc_profile(flag, "--deepness list[%p]--", a_conf->deepness);
		zc_arraylist_foreach(a_conf->deepness, i, a_deepness) {
			zlog_deepness_profile(a_deepness, flag);
		}
	}

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
	if (a_conf->default_deepness_str) zc_sdsfree(a_conf->default_deepness_str);
	if (a_conf->default_format_str) zc_sdsfree(a_conf->default_format_str);

	if (a_conf->rotater) zlog_rotater_del(a_conf->rotater);
	if (a_conf->default_deepness) zlog_deepness_del(a_conf->default_deepness);
	if (a_conf->default_format) zlog_format_del(a_conf->default_format);

	if (a_conf->deepness) zc_arraylist_del(a_conf->deepness);
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

	a_conf->default_format_str = zc_sdsnew(ZLOG_CONF_DEFAULT_FORMAT);
	if (!a_conf->default_format_str) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }
	a_conf->default_deepness_str = zc_sdsnew(ZLOG_CONF_DEFAULT_DEEPNESS);
	if (!a_conf->default_deepness_str) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }
	a_conf->rotate_lock_file = zc_sdsnew(ZLOG_CONF_BACKUP_ROTATE_LOCK_FILE);
	if (!a_conf->rotate_lock_file) { zc_error("zc_sdsnew fail, errno[%d]", errno); goto err; }

	a_conf->deepness = zc_arraylist_new((zc_arraylist_del_fn) zlog_deepness_del);
	if (!a_conf->deepness) { zc_error("zc_arraylist_new fail"); goto err; }
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
		a_conf->rotate_lock_file = zc_sdscpy(a_conf->rotate_lock_file , a_conf->file);
		if (!a_conf->rotate_lock_file) { zc_error("zc_sdscpy fail, errno[%d]", errno); goto err; }

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

	a_conf->default_format = zlog_format_new(a_conf->default_format_str, &(a_conf->time_spec_count));
	if (!a_conf->default_format) { zc_error("zlog_format_new fail"); return -1; }

	a_conf->default_deepness = zlog_deepness_new(a_conf->default_deepness_str);
	if (!a_conf->default_deepness) { zc_error("zlog_deepness_new fail"); return -1; }

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
	int section = 0; /* [global:1] [deepness:2] [levels:3] [formats:4] [rules:5] */

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

/* section [global:1] [deepness:2] [levels:3] [formats:4] [rules:5] */
static int zlog_conf_parse_line(zlog_conf_t * a_conf, zc_sds line, int *section)
{
	int argc;
	zc_sds *argv = NULL;
	zlog_format_t *a_format = NULL;
	zlog_deepness_t *a_deepness = NULL;
	zlog_rule_t *a_rule = NULL;

	line = zc_sdstrim(line, " \t\r\n");

	/* process section tag */
	if (line[0] == '[') {
		int last_section = *section;

		line = zc_sdstrim(line, "[] \t\r\n");
		if (STRICMP(line, ==, "global")) {
			*section = 1;
		} else if (STRICMP(line, ==, "deepness")) {
			*section = 2;
		} else if (STRICMP(line, ==, "levels")) {
			*section = 3;
		} else if (STRICMP(line, ==, "formats")) {
			*section = 4;
		} else if (strchr(line, '.') != NULL) {
			*section = 5;
		} else {
			zc_error("wrong section name[%s]", line);
			goto err;
		}

		/* check the sequence of section, must increase */
		if (last_section > *section) {
			zc_error("wrong sequence of section, must follow global->deepness->levels->formats->rules");
			goto err;
		}

		if (*section == 5) {
			/* now build rotater and default_format when setcion 4 [rules] starts
			 * from the unchanging global setting,
			 * for zlog_rule_new() */
			a_conf->rotater = zlog_rotater_new(a_conf->rotate_lock_file);	
			if (!a_conf->rotater) { zc_error("zlog_rotater_new fail"); goto err; }

			a_conf->default_deepness = zlog_deepness_new(a_conf->default_deepness_str);
			if (!a_conf->default_deepness) { zc_error("zlog_deepness_new fail"); goto err; }

			a_conf->default_format = zlog_format_new(a_conf->default_format_str, &(a_conf->time_spec_count));
			if (!a_conf->default_format) { zc_error("zlog_format_new fail"); goto err; }

		}
		goto exit;
	} 

	argv = zc_sdssplitargs(line, "=", &argc);
	if (!argv) { zc_error("Unbalanced quotes in configuration line"); goto err; }

	/* process detail */
	switch (*section) {
	case 1:
		if (STRICMP(argv[0], ==, " strict") && STRICMP(argv[1], ==, " init")) {
			/* environment variable ZLOG_STRICT_INIT > conf setting */
			if (STRICMP(argv[2], ==, " false") && !getenv("ZLOG_STRICT_INIT")) {
				a_conf->strict_init = 0;
			} else {
				a_conf->strict_init = 1;
			}
		} else if (STRICMP(argv[0], ==, " rotate") && STRICMP(argv[1], ==, " lock")
				&& STRICMP(argv[2], ==, " file")) {
			/* may overwrite the inner default value, or last value */

			a_conf->rotate_lock_file = zc_sdscpy(a_conf->rotate_lock_file, 
				STRICMP(argv[3], ==, " self") ?  a_conf->file : argv[3] + 1);
			if (!a_conf->rotate_lock_file) { zc_error("zc_sdscpy fail, errno[%d]", errno); goto err; }

		} else if (STRICMP(argv[0], ==, " auto") && STRICMP(argv[1], ==, " reload")) {
			int err;
			a_conf->auto_reload = zc_strtoz(argv[2] + 1, &err);
			if (err) {zc_error("zc_strtoz fail"); goto err; }
		} else {
			zc_error("[%s] is not any one of global options", line);
			goto err;
		}
		break;
	case 2:
		if (STRCMP(argv[0], ==, " *")) {
			zc_sdscpy(a_conf->default_deepness_str, line);
		} else {
			a_deepness = zlog_deepness_new(line);
			if (!a_deepness) { zc_error("zlog_deepness_new fail [%s]", line); goto err; }
			if (zc_arraylist_add(a_conf->deepness, a_deepness)) {
				zc_error("zc_arraylist_add fail");
				zlog_deepness_del(a_deepness);
				goto err;
			}
		}
		break;
	case 3:
		if (zlog_level_list_set(a_conf->levels, line)) {
			zc_error("zlog_level_list_set fail");
			goto err;
		}
		break;
	case 4:
		if (STRCMP(argv[0], ==, " *")) {
			zc_sdscpy(a_conf->default_format_str, line);
		} else {
			a_format = zlog_format_new(line , &(a_conf->time_spec_count));
			if (!a_format) { zc_error("zlog_format_new fail [%s]", line); goto err; }
			if (zc_arraylist_add(a_conf->formats, a_format)) {
				zlog_format_del(a_format);
				zc_error("zc_arraylist_add fail");
				goto err;
			}
		}
		break;
	case 5:
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
