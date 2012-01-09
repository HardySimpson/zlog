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

/**
 * @file zlog.h
 * @brief zlog interface
 *
 * It is the only file need to be included for daily use of zlog.
 */

#include <stdarg.h>

/**
 * Initialise zlog from configure file.
 * 
 * zlog_init() shall use once per process, in any threads.
 * It will also create a category's table, a rotater for rotate too large log file,
   a threads buf table.
 * Only 1st time use zlog_init() is effective, other times will record a error log in
   ZLOG_ERROR_LOG, and return -1.
 * Can use zlog_update() to reload configure file.
 * Before exit program, use zlog_fini() to release all memory zlog_init() applied.
 * 
 * @param conf_file configure file path, both relative path and absolute path is accept.
    If conf_file is NULL, use environment variable ZLOG_CONFPATH instead, else if
    ZLOG_CONFPATH is NULL, use /etc/zlog.conf instead.
 * @returns 0 for success, -1 for fail, detail will be record in ZLOG_ERROR_LOG.
 * @see zlog_update(), zlog_fini(), zlog.conf, ZLOG_ERROR_LOG
 */
	int zlog_init(char *conf_file);

/**
 * Update zlog from configure file.
 *
 * After zlog_init(), zlog_update() shall reread configure file.
 * Can be use at any time configure file changed, and can be use unlimit times.
 * It shall recaculate output rules of all categories, rebuild buffer of each thread,
   reopen the new lock file for rotater.
 *
 * @param conf_file configure file path, both relative path and absolute path is accept.
    If conf_file is NULL, use the last configure file zlog_init() or zlog_update() specified.
 * @returns 0 for success, -1 for fail, detail will be record in ZLOG_ERROR_LOG.
 * @see zlog_init(), zlog_fini(), zlog.conf, ZLOG_ERROR_LOG
 */
	int zlog_update(char *conf_file);

/**
 * Finish zlog, release all memory zlog_init() or zlog_update() applied.
 * 
 * @see zlog_init(), zlog_update(), zlog.conf, ZLOG_ERROR_LOG
 */
	void zlog_fini(void);

/**
 * Output detail of zlog's configure file to ZLOG_ERROR_LOG.
 *
 * The function is always used when u are not sure what is zlog's conf now.
 *
 * @see ZLOG_ERROR_LOG
 */
	void zlog_profile(void);

/**
 * Category is the core concept of zlog.
 *
 * Not like logger of log4j, category in zlog doesn't have heritage.
 * A category has a name and several rules from confiugre file. 
 * For example, a category which name is "aa_bb_cc" match rules below:
 *
 * @code
 * aa_.INFO             >stderr
 * aa_bb_.DEBUG         "/var/log/debug.log"
 * aa_bb_cc.ERROR       "/var/log/$c.log"
 * @endcode
 *
 * But this category not match:
 * @code
 * aa_bb.*              >stdout
 * @endcode
 *
 * @see zlog_rule_match_category()
 */
	typedef struct zlog_category_t zlog_category_t;

/**
 * Get a category from global table for future log, if none, create it.
 *
 * @param category_name category's name, must consist of alpha, digit or _.
    The length of category_name shall not be longer than PATH_MAX from limits.h,
    which is commonly 4096 on linux system.
 * @returns a zlog_category_t pointer for success,
    NULL for fail, detail will be record in ZLOG_ERROR_LOG.
 * @see zlog_set_category(), zlog_category_t, ZLOG_ERROR_LOG
 */
	zlog_category_t *zlog_get_category(char *category_name);

/**
 * put key-value into mdc of thread now, mdc is come from log4j, Mapped Diagnostic Context,
 * correspond to $M(key) in configure file
 */
	int zlog_put_mdc(char *key, char *value);

/**
 * get value from mdc of thread now
 */
	char *zlog_get_mdc(char *key);

/**
 * remove key-value from mdc of thread now
 */
	void zlog_remove_mdc(char *key);

/**
 * remove all values from mdc of thread now
 */
	void zlog_clean_mdc(void);

/**
 * The real log fuction.
 *
 * Generaly use ZLOG_XXX macro, it is for super user.
 * In the begining of each thread of one process, it will be a litte slow,
 * because zlog will malloc space and of buffer and event for the thread.
 *
 * @param a_cat the output category pointer, shall not be NULL.
 * @param file C source file name, is always fill with __FILE__ macro.
 * @param line C source file name, is always fill with __LINE__ macro.
 * @param priority one priority from zlog_priority, if <1 or >6,
    this message will be record as ZLOG_UNKOWN.
 * @param format C printf style format, if use a wrong format like "%l",
    nothing will be output, and record a log in ZLOG_ERROR_LOG.
 * @see vzlog(), hzlog()
 */
	void zlog(zlog_category_t * a_cat, char *file, long line, int priority,
		  char *format, ...);

/**
 * The real log fuction, va_list version.
 *
 * Generaly use VZLOG_XXX macro, it is for super user.
 *
 * @param a_cat the output category pointer, shall not be NULL.
 * @param file C source file name, is always fill with __FILE__ macro.
 * @param line C source file name, is always fill with __LINE__ macro.
 * @param priority one priority from zlog_priority, if <1 or >6,
    this message will be record as ZLOG_UNKOWN.
 * @param format C printf style format, if use a wrong format like "%l",
    nothing will be output, and record a log in ZLOG_ERROR_LOG.
 * @param args a va_list, will be va_copy() inner.
 */
	void vzlog(zlog_category_t * a_cat, char *file, long line, int priority,
		   char *format, va_list args);

/**
 * The real log fuction, output hexadecimal version.
 *
 * Generaly use HZLOG_XXX macro, it is for super user.
 * The output looks like:
 * @code
 * 11-15 13:44:21 DEBUG [10670:test_hex.c:89] hex_buf_len=[5365]
 *              0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF
 * 0000000001   23 21 20 2f 62 69 6e 2f 62 61 73 68 0a 0a 23 20   #! /bin/bash..# 
 * 0000000002   74 65 73 74 5f 68 65 78 20 2d 20 74 65 6d 70 6f   test_hex - tempo
 * 0000000003   72 61 72 79 20 77 72 61 70 70 65 72 20 73 63 72   rary wrapper scr
 * @endcode
 *
 * @param a_cat the output category pointer, shall not be NULL
 * @param file C source file name, is always fill with __FILE__ macro
 * @param line C source file name, is always fill with __LINE__ macro
 * @param priority one priority from zlog_priority, if <1 or >6,
    this message will be record as ZLOG_UNKOWN
 * @param buf the buf start pointer
 * @param buf_len buf's length
 */
	void hzlog(zlog_category_t * a_cat, char *file, long line, int priority,
		   char *buf, unsigned long buf_len);

	typedef enum {
		ZLOG_UNKOWN = 0,/**< when priority < 1 or > 6 */
		ZLOG_DEBUG = 1,	/**< debug-level message */
		ZLOG_INFO = 2,	/**< informational message */
		ZLOG_NOTICE = 3,/**< normal, but significant, condition */
		ZLOG_WARN = 4,	/**< warning conditions, maybe application logic problem */
		ZLOG_ERROR = 5,	/**< error conditions, maybe application fail */
		ZLOG_FATAL = 6,	/**< system is unusable */
	} zlog_priority;

/* zlog macros */

#define ZLOG_FATAL(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_FATAL, format, ##args)

#define ZLOG_ERROR(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_ERROR, format, ##args)

#define ZLOG_WARN(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_WARN, format, ##args)

#define ZLOG_NOTICE(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_NOTICE, format, ##args)

#define ZLOG_INFO(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_INFO, format, ##args)

#define ZLOG_DEBUG(cat, format, args...) \
	zlog(cat, __FILE__, __LINE__, ZLOG_DEBUG, format, ##args)

/* vzlog macros */

#define VZLOG_FATAL(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_FATAL, format, args)

#define VZLOG_ERROR(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_ERROR, format, args)

#define VZLOG_WARN(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_WARN, format, args)

#define VZLOG_NOTICE(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_NOTICE, format, args)

#define VZLOG_INFO(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_INFO, format, args)

#define VZLOG_DEBUG(cat, format, args) \
	vzlog(cat, __FILE__, __LINE__, ZLOG_DEBUG, format, args)

/* hzlog macros */
#define HZLOG_FATAL(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_FATAL, buf, buf_len)

#define HZLOG_ERROR(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_ERROR, buf, buf_len)

#define HZLOG_WARN(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_WARN, buf, buf_len)

#define HZLOG_NOTICE(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_NOTICE, buf, buf_len)

#define HZLOG_INFO(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_INFO, buf, buf_len)

#define HZLOG_DEBUG(cat, buf, buf_len) \
	hzlog(cat, __FILE__, __LINE__, ZLOG_DEBUG, buf, buf_len)

#ifdef __cplusplus
}
#endif
#endif
