#ifndef __xlog_event_h
#define __xlog_event_h

/**
 * @file event.h
 * @brief xlog event class, keep all infomation of one message
    which comes from once user log function.
 */

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include "zc_defs.h"

/**
 * control how the log to be writen, hexadecimal or by printf format
 */
typedef enum {
	XLOG_FMT = 0,
	XLOG_HEX = 1,
} xlog_event_cmd;

/**
 * xlog event struct
 */
typedef struct {
	char *category_name;		/**< point to a category.name */
	size_t *category_name_len;	/**< point to a category.name_len */
	char host_name[256 + 1];	/**< fill by gethostname */
	size_t host_name_len;		/**< strlen(hostname) */

	char *file;			/**< source file name */
	long line;			/**< source file line */
	int priority;			/**< priority of the log this time */

	char *hex_buf;			/**< if use hxlog(), point to user's hex_buf */
	long hex_buf_len;		/**< strlen(hex_buf) */
	char *str_format;		/**< if use xlog() or vxlog(), printf format */
	va_list str_args;		/**< if use xlog() or vxlog(), printf arguements */
	xlog_event_cmd generate_cmd;	/**< cmd */

	struct timeval time_stamp;	/**< will be assign when needed */
	struct tm local_time;		/**< local time of time_stamp */
	char ms[3 + 1];			/**< millisecond */
	char us[6 + 1];			/**< microsecond */
	char time_fmt_msus[MAXLEN_CFG_LINE + 1];
					/**< a_spec->time_fmt + ms||us = time_fmt for thread&now */

	pid_t pid;			/**< process id, getpid() */
	pthread_t tid;			/**< thread id, pthread_self */
} xlog_event_t;

/**
 * xlog_event_t constructor
 *
 * @returns xlog_event_t pointer for success, NULL for fail
 */
xlog_event_t *xlog_event_new(void);

/**
 * xlog_event_t destructor
 *
 * @param xlog_event_t pointer
 */
void xlog_event_del(xlog_event_t *a_event);

/**
 * xlog_event_t setter
 *
 * @see xlog_event_t
 */
void xlog_event_set(xlog_event_t * a_event,
		    char *category_name, size_t *category_name_len,
		    char *file, long line, int priority,
		    char *hex_buf, long hex_buf_len, char *str_format, va_list str_args, int generate_cmd);

#endif
