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

/**
 * @defgroup interface zlog interface
 * @{
 */

#ifndef __zlog_h
#define __zlog_h

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file zlog.h
 * @brief It is the only file need to be included for daily use of zlog.
 */

#include <stdarg.h>

/**
 * @brief Initialise buffers, rules, formats and open lock file according to configure file.
 * 
 * zlog_init() shall use once per process, in any threads.
 * It will also create a category's table, a rotater for rotate too large log file,
   a threads buf table.
 * Only 1st time use zlog_init() is effective, other times will record a error log in
   ZLOG_PROFILE_ERROR, and return -1.
 * Use zlog_update() to reload configure file.
 * Before exit program, use zlog_fini() to release all memory zlog_init() applied.
 * 
 * @param conf_file configure file path, both relative path and absolute path is accept.
    If conf_file is NULL, use environment variable ZLOG_CONFPATH instead, else if
    ZLOG_CONF_PATH is NULL, output all logs to stdout.
 * @returns 0 for success, -1 for fail, detail will be record in ZLOG_PROFILE_ERROR.
 * @see zlog_update(), zlog_fini(), zlog.conf, ZLOG_PROFILE_ERROR
 */
	int zlog_init(char *conf_file);

/**
 * @brief Update buffers, rules, formats and reopen lock file according to configure file.
 *
 * After zlog_init(), zlog_update() shall reread configure file.
 * Can be use at any time configure file changed, and can be use unlimit times.
 * It shall recaculate output rules of all categories, rebuild buffer of each thread,
   reopen the new lock file for rotater.
 *
 * @param conf_file configure file path, both relative path and absolute path is accept.
    If conf_file is NULL, use the last configure file zlog_init() or zlog_update() specified.
 * @returns 0 for success, -1 for fail, detail will be record in ZLOG_PROFILE_ERROR.
 * @see zlog_init(), zlog_fini(), zlog.conf, ZLOG_PROFILE_ERROR
 */
	int zlog_update(char *conf_file);

/**
 * @brief Finish zlog, release all memory zlog_init() or zlog_update() applied.
 * 
 * @see zlog_init(), zlog_update(), zlog.conf, ZLOG_PROFILE_ERROR
 */
	void zlog_fini(void);

/**
 * @brief Output detail of zlog's configure file to ZLOG_PROFILE_ERROR.
 *
 * The function is always used when u are not sure what is zlog's conf now.
 *
 * @see ZLOG_PROFILE_ERROR
 */
	void zlog_profile(void);

/**
 * @brief Points to a structure that contains rules from configure file.
 *
 * Not like logger of log4j, category in zlog doesn't have heritage.
 * For example, a category which name is "aa_bb_cc" match rules below:
 *
 * @code
 * aa_.INFO             >stderr
 * aa_bb_.DEBUG         "/var/log/debug.log"
 * aa_bb_cc.ERROR       "/var/log/%c.log"
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
 * @brief Get a category from global table for future log, if none, create it.
 *
 * @param category_name category's name, must consist of alpha, digit or _.
    The length of category_name shall not be longer than PATH_MAX from limits.h,
    which is commonly 4096 on linux system.
 * @returns a zlog_category_t pointer for success,
    NULL for fail, detail will be record in ZLOG_PROFILE_ERROR.
 * @see zlog_category_t, ZLOG_PROFILE_ERROR
 */
	zlog_category_t *zlog_get_category(char *category_name);

/**
 * @brief Put key-value into a map(one thread per map), can be show in log
 *
 * mdc , Mapped Diagnostic Context,
 * correspond to %M(key) in configure file
 */
	int zlog_put_mdc(char *key, char *value);

/**
 * @brief Get value from mdc of thread now
 */
	char *zlog_get_mdc(char *key);

/**
 * @brief Remove key-value from mdc of thread now
 */
	void zlog_remove_mdc(char *key);

/**
 * @brief Remove all values from mdc of thread now
 */
	void zlog_clean_mdc(void);

/**
 * @brief The real log fuction.
 *
 * Generaly use ZLOG_XXX macro, it is for super user.
 * In the begining of each thread of one process, it will be a litte slow,
 * because zlog will malloc space and of buffer and event for the thread.
 *
 * @param a_cat the output category pointer, shall not be NULL.
 * @param file C source file name, is always fill with __FILE__ macro.
 * @param line C source file name, is always fill with __LINE__ macro.
 * @param level one level from zlog_level, if <=0 or >=255,
    this message will be record as ZLOG_UNKOWN.
 * @param format C printf style format, if use a wrong format like "%l",
    nothing will be output, and record a log in ZLOG_PROFILE_ERROR.
 * @see vzlog(), hzlog()
 */
	void zlog(zlog_category_t * a_cat, char *file, long line, int level,
		  char *format, ...);

/**
 * @brief The real log fuction, va_list version.
 *
 * Generaly use VZLOG_XXX macro, it is for super user.
 *
 * @param a_cat the output category pointer, shall not be NULL.
 * @param file C source file name, is always fill with __FILE__ macro.
 * @param line C source file name, is always fill with __LINE__ macro.
 * @param level one level from zlog_level, if <=0 or >=255,
    this message will be record as ZLOG_UNKOWN.
 * @param format C printf style format, if use a wrong format like "%l",
    nothing will be output, and record a log in ZLOG_PROFILE_ERROR.
 * @param args a va_list, will be va_copy() inner.
 */
	void vzlog(zlog_category_t * a_cat, char *file, long line, int level,
		   char *format, va_list args);

/**
 * @brief The real log fuction, output hexadecimal version.
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
 * @param level one level from zlog_level, if <=0 or >=255,
    this message will be record as ZLOG_UNKOWN
 * @param buf the buf start pointer
 * @param buf_len buf's length
 */
	void hzlog(zlog_category_t * a_cat, char *file, long line, int level,
		   void *buf, size_t buf_len);

/******* useful macros, can be redefined at user's h file **********/

	typedef enum {
		ZLOG_LEVEL_DEBUG = 20,	/**< debug-level message */
		ZLOG_LEVEL_INFO = 40,	        /**< informational message */
		ZLOG_LEVEL_NOTICE = 60,       /**< normal, but significant, condition */
		ZLOG_LEVEL_WARN = 80,	        /**< warning conditions, maybe application logic problem */
		ZLOG_LEVEL_ERROR = 100,	/**< error conditions, maybe application fail */
		ZLOG_LEVEL_FATAL = 120,	/**< system is unusable */
	} zlog_level;

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

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
