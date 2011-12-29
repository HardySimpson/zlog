/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __xlog_h
#define __xlog_h

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file xlog.h
 * @brief xlog interface
 *
 * It is the only file need to be included for daily use of xlog.
 */

#include <stdarg.h>

/**
 * Initialise xlog from configure file.
 * 
 * xlog_init() shall use once per process, in any threads.
 * It will also create a category's table, a rotater for rotate too large log file,
   a threads buf table.
 * Only 1st time use xlog_init() is effective, other times will record a error log in
   XLOG_ERROR_LOG, and return -1.
 * Can use xlog_update() to reload configure file.
 * Before exit program, use xlog_fini() to release all memory xlog_init() applied.
 * 
 * @param conf_file configure file path, both relative path and absolute path is accept.
    If conf_file is NULL, use environment variable XLOG_CONFPATH instead, else if
    XLOG_CONFPATH is NULL, use /etc/xlog.conf instead.
 * @returns 0 for success, -1 for fail, detail will be record in XLOG_ERROR_LOG.
 * @see xlog_update(), xlog_fini(), xlog.conf, XLOG_ERROR_LOG
 */
int xlog_init(char *conf_file);

/**
 * Update xlog from configure file.
 *
 * After xlog_init(), xlog_update() shall reread configure file.
 * Can be use at any time configure file changed, and can be use unlimit times.
 * It shall recaculate output rules of all categories, rebuild buffer of each thread,
   reopen the new lock file for rotater.
 *
 * @param conf_file configure file path, both relative path and absolute path is accept.
    If conf_file is NULL, use the last configure file xlog_init() or xlog_update() specified.
 * @returns 0 for success, -1 for fail, detail will be record in XLOG_ERROR_LOG.
 * @see xlog_init(), xlog_fini(), xlog.conf, XLOG_ERROR_LOG
 */
int xlog_update(char *conf_file);

/**
 * Finish xlog, release all memory xlog_init() or xlog_update() applied.
 * 
 * @see xlog_init(), xlog_update(), xlog.conf, XLOG_ERROR_LOG
 */
void xlog_fini(void);

/**
 * Output detail of xlog's configure file to XLOG_ERROR_LOG.
 *
 * The function is always used when u are not sure what is xlog's conf now.
 *
 * @see XLOG_ERROR_LOG
 */
void xlog_profile(void);

/**
 * Category is the core concept of xlog.
 *
 * Not like logger of log4j, category in xlog doesn't have heritage.
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
 * @see xlog_rule_match_category()
 */
typedef struct xlog_category_t xlog_category_t;

/**
 * Get a category from global table for future log, if none, create it.
 *
 * @param category_name category's name, must consist of alpha, digit or _.
    The length of category_name shall not be longer than PATH_MAX from limits.h,
    which is commonly 4096 on linux system.
 * @returns a xlog_category_t pointer for success,
    NULL for fail, detail will be record in XLOG_ERROR_LOG.
 * @see xlog_set_category(), xlog_category_t, XLOG_ERROR_LOG
 */
xlog_category_t *xlog_get_category(char *category_name);

/**
 * put key-value into mdc of thread now, mdc is come from log4j, Mapped Diagnostic Context,
 * correspond to $M(key) in configure file
 */
int xlog_put_mdc(char *key, char *value);

/**
 * get value from mdc of thread now
 */
char *xlog_get_mdc(char *key);

/**
 * remove key-value from mdc of thread now
 */
void xlog_remove_mdc(char *key);

/**
 * remove all values from mdc of thread now
 */
void xlog_clean_mdc(void);

/**
 * The real log fuction.
 *
 * Generaly use XLOG_XXX macro, it is for super user.
 * In the begining of each thread of one process, it will be a litte slow,
 * because xlog will malloc space and of buffer and event for the thread.
 *
 * @param a_cat the output category pointer, shall not be NULL.
 * @param file C source file name, is always fill with __FILE__ macro.
 * @param line C source file name, is always fill with __LINE__ macro.
 * @param priority one priority from xlog_priority, if <1 or >6,
    this message will be record as XLOG_UNKOWN.
 * @param format C printf style format, if use a wrong format like "%l",
    nothing will be output, and record a log in XLOG_ERROR_LOG.
 * @see vxlog(), hxlog()
 */
void xlog(xlog_category_t *a_cat, char *file, long line, int priority, char *format, ...);

/**
 * The real log fuction, va_list version.
 *
 * Generaly use VXLOG_XXX macro, it is for super user.
 *
 * @param a_cat the output category pointer, shall not be NULL.
 * @param file C source file name, is always fill with __FILE__ macro.
 * @param line C source file name, is always fill with __LINE__ macro.
 * @param priority one priority from xlog_priority, if <1 or >6,
    this message will be record as XLOG_UNKOWN.
 * @param format C printf style format, if use a wrong format like "%l",
    nothing will be output, and record a log in XLOG_ERROR_LOG.
 * @param args a va_list, will be va_copy() inner.
 */
void vxlog(xlog_category_t *a_cat, char *file, long line, int priority, char *format, va_list args);

/**
 * The real log fuction, output hexadecimal version.
 *
 * Generaly use HXLOG_XXX macro, it is for super user.
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
 * @param priority one priority from xlog_priority, if <1 or >6,
    this message will be record as XLOG_UNKOWN
 * @param buf the buf start pointer
 * @param buf_len buf's length
 */
void hxlog(xlog_category_t *a_cat, char *file, long line, int priority, char *buf, unsigned long buf_len);

typedef enum {
	XLOG_UNKOWN = 0,	/**< when priority < 1 or > 6 */
	XLOG_DEBUG = 1,		/**< debug-level message */
	XLOG_INFO = 2,		/**< informational message */
	XLOG_NOTICE = 3,	/**< normal, but significant, condition */ 
	XLOG_WARN = 4,		/**< warning conditions, maybe application logic problem */
	XLOG_ERROR = 5,		/**< error conditions, maybe application fail */
	XLOG_FATAL = 6,		/**< system is unusable */
} xlog_priority;

/* xlog macros */

#define XLOG_FATAL(cat, format, args...) \
	xlog(cat, __FILE__, __LINE__, XLOG_FATAL, format, ##args)

#define XLOG_ERROR(cat, format, args...) \
	xlog(cat, __FILE__, __LINE__, XLOG_ERROR, format, ##args)

#define XLOG_WARN(cat, format, args...) \
	xlog(cat, __FILE__, __LINE__, XLOG_WARN, format, ##args)

#define XLOG_NOTICE(cat, format, args...) \
	xlog(cat, __FILE__, __LINE__, XLOG_NOTICE, format, ##args)

#define XLOG_INFO(cat, format, args...) \
	xlog(cat, __FILE__, __LINE__, XLOG_INFO, format, ##args)

#define XLOG_DEBUG(cat, format, args...) \
	xlog(cat, __FILE__, __LINE__, XLOG_DEBUG, format, ##args)

/* vxlog macros */

#define VXLOG_FATAL(cat, format, args) \
	vxlog(cat, __FILE__, __LINE__, XLOG_FATAL, format, args)

#define VXLOG_ERROR(cat, format, args) \
	vxlog(cat, __FILE__, __LINE__, XLOG_ERROR, format, args)

#define VXLOG_WARN(cat, format, args) \
	vxlog(cat, __FILE__, __LINE__, XLOG_WARN, format, args)

#define VXLOG_NOTICE(cat, format, args) \
	vxlog(cat, __FILE__, __LINE__, XLOG_NOTICE, format, args)

#define VXLOG_INFO(cat, format, args) \
	vxlog(cat, __FILE__, __LINE__, XLOG_INFO, format, args)

#define VXLOG_DEBUG(cat, format, args) \
	vxlog(cat, __FILE__, __LINE__, XLOG_DEBUG, format, args)

/* hxlog macros */
#define HXLOG_FATAL(cat, buf, buf_len) \
	hxlog(cat, __FILE__, __LINE__, XLOG_FATAL, buf, buf_len)

#define HXLOG_ERROR(cat, buf, buf_len) \
	hxlog(cat, __FILE__, __LINE__, XLOG_ERROR, buf, buf_len)

#define HXLOG_WARN(cat, buf, buf_len) \
	hxlog(cat, __FILE__, __LINE__, XLOG_WARN, buf, buf_len)

#define HXLOG_NOTICE(cat, buf, buf_len) \
	hxlog(cat, __FILE__, __LINE__, XLOG_NOTICE, buf, buf_len)

#define HXLOG_INFO(cat, buf, buf_len) \
	hxlog(cat, __FILE__, __LINE__, XLOG_INFO, buf, buf_len)

#define HXLOG_DEBUG(cat, buf, buf_len) \
	hxlog(cat, __FILE__, __LINE__, XLOG_DEBUG, buf, buf_len)

#ifdef __cplusplus
}
#endif

#endif
