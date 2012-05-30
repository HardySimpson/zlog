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

#ifndef __zlog_h
#define __zlog_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

int zlog_init(char *conf_file);
int dzlog_init(char *conf_file, char *default_category_name);
int zlog_reload(char *conf_file);
void zlog_fini(void);
void zlog_profile(void);

typedef struct zlog_category_t zlog_category_t;
zlog_category_t *zlog_get_category(char *category_name);
int dzlog_set_category(char *default_category_name);

int zlog_put_mdc(char *key, char *value);
char *zlog_get_mdc(char *key);
void zlog_remove_mdc(char *key);
void zlog_clean_mdc(void);

void zlog(zlog_category_t * a_cat, char *file, long line, int level,
		  char *format, ...);
void dzlog(char *file, long line, int level, char *format, ...);

void vzlog(zlog_category_t * a_cat, char *file, long line, int level,
	   char *format, va_list args);
void vdzlog(char *file, long line, int level,
		   char *format, va_list args);

void hzlog(zlog_category_t * a_cat, char *file, long line, int level,
		   void *buf, size_t buf_len);
void hdzlog(char *file, long line, int level,
	   void *buf, size_t buf_len);

/******* useful macros, can be redefined at user's h file **********/

typedef enum {
	ZLOG_LEVEL_DEBUG = 20,
	ZLOG_LEVEL_INFO = 40,
	ZLOG_LEVEL_NOTICE = 60,
	ZLOG_LEVEL_WARN = 80,
	ZLOG_LEVEL_ERROR = 100,
	ZLOG_LEVEL_FATAL = 120,
} zlog_level;

#if defined __STDC_VERSION__ && STDC_VERSION__ > 199901
/* zlog macros */
#define ZLOG_FATAL(cat, format, ...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_FATAL, format, ## __VA_ARGS__)
#define ZLOG_ERROR(cat, format, ...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_ERROR, format, ## __VA_ARGS__)
#define ZLOG_WARN(cat, format, ...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_WARN, format, ## __VA_ARGS__)
#define ZLOG_NOTICE(cat, format, ...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_NOTICE, format, ## __VA_ARGS__)
#define ZLOG_INFO(cat, format, ...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define ZLOG_DEBUG(cat, format, ...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_DEBUG, format, ## __VA_ARGS__)

/* dzlog macros */
#define DZLOG_FATAL(format, ...) \
	ddzlog(__FILE__, __LINE__, ZLOG_LEVEL_FATAL, format, ## __VA_ARGS__)
#define DZLOG_ERROR(format, ...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_ERROR, format, ## __VA_ARGS__)
#define DZLOG_WARN(format, ...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_WARN, format, ## __VA_ARGS__)
#define DZLOG_NOTICE(format, ...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_NOTICE, format, ## __VA_ARGS__)
#define DZLOG_INFO(format, ...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define DZLOG_DEBUG(format, ...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_DEBUG, format, ## __VA_ARGS__)

#elif defined __GNUC__
/* zlog macros */
#define ZLOG_FATAL(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_FATAL, format, ##args)
#define ZLOG_ERROR(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_ERROR, format, ##args)
#define ZLOG_WARN(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_WARN, format, ##args)
#define ZLOG_NOTICE(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_NOTICE, format, ##args)
#define ZLOG_INFO(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_INFO, format, ##args)
#define ZLOG_DEBUG(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_DEBUG, format, ##args)

/* dzlog macros */
#define DZLOG_FATAL(format, args...) \
	ddzlog(__FILE__, __LINE__, ZLOG_LEVEL_FATAL, format, ##args)
#define DZLOG_ERROR(format, args...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_ERROR, format, ##args)
#define DZLOG_WARN(format, args...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_WARN, format, ##args)
#define DZLOG_NOTICE(format, args...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_NOTICE, format, ##args)
#define DZLOG_INFO(format, args...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_INFO, format, ##args)
#define DZLOG_DEBUG(format, args...) \
	dzlog(__FILE__, __LINE__, ZLOG_LEVEL_DEBUG, format, ##args)
#endif

/* vzlog macros */
#define VZLOG_FATAL(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_FATAL, format, args)
#define VZLOG_ERROR(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_ERROR, format, args)
#define VZLOG_WARN(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_WARN, format, args)
#define VZLOG_NOTICE(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_NOTICE, format, args)
#define VZLOG_INFO(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_INFO, format, args)
#define VZLOG_DEBUG(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_DEBUG, format, args)

/* hzlog macros */
#define HZLOG_FATAL(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_FATAL, buf, buf_len)
#define HZLOG_ERROR(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_ERROR, buf, buf_len)
#define HZLOG_WARN(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_WARN, buf, buf_len)
#define HZLOG_NOTICE(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_NOTICE, buf, buf_len)
#define HZLOG_INFO(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_INFO, buf, buf_len)
#define HZLOG_DEBUG(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_LEVEL_DEBUG, buf, buf_len)

/* vdzlog macros */
#define VDZLOG_FATAL(format, args) \
	vdzlog(__FILE__, __LINE__, ZLOG_LEVEL_FATAL, format, args)
#define VDZLOG_ERROR(format, args) \
	vdzlog(__FILE__, __LINE__, ZLOG_LEVEL_ERROR, format, args)
#define VDZLOG_WARN(format, args) \
	vdzlog(__FILE__, __LINE__, ZLOG_LEVEL_WARN, format, args)
#define VDZLOG_NOTICE(format, args) \
	vdzlog(__FILE__, __LINE__, ZLOG_LEVEL_NOTICE, format, args)
#define VDZLOG_INFO(format, args) \
	vdzlog(__FILE__, __LINE__, ZLOG_LEVEL_INFO, format, args)
#define VDZLOG_DEBUG(format, args) \
	vdzlog(__FILE__, __LINE__, ZLOG_LEVEL_DEBUG, format, args)

/* hdzlog macros */
#define HDZLOG_FATAL(buf, buf_len) \
	hdzlog(__FILE__, __LINE__, ZLOG_LEVEL_FATAL, buf, buf_len)
#define HDZLOG_ERROR(buf, buf_len) \
	hdzlog(__FILE__, __LINE__, ZLOG_LEVEL_ERROR, buf, buf_len)
#define HDZLOG_WARN(buf, buf_len) \
	hdzlog(__FILE__, __LINE__, ZLOG_LEVEL_WARN, buf, buf_len)
#define HDZLOG_NOTICE(buf, buf_len) \
	hdzlog(__FILE__, __LINE__, ZLOG_LEVEL_NOTICE, buf, buf_len)
#define HDZLOG_INFO(buf, buf_len) \
	hdzlog(__FILE__, __LINE__, ZLOG_LEVEL_INFO, buf, buf_len)
#define HDZLOG_DEBUG(buf, buf_len) \
	hdzlog(__FILE__, __LINE__, ZLOG_LEVEL_DEBUG, buf, buf_len)

#ifdef __cplusplus
}
#endif

#endif
