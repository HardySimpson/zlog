#ifndef __xlog_priority_h
#define __xlog_priority_h

/**
 * @file priority.h
 * @brief xlog priority functions
 */

/**
 * make 0-5 to "UNKNOWN", "DEBUG"...
 *
 * @param priority int priority
 * @returns const static string of priority
 */
char *xlog_priority_itostr(int priority);

/**
 * make "UNKNOWN", "DEBUG"... to 0-5
 *
 * @param string of priority
 * @returns priority int priority
 */
int xlog_priority_strtoi(char *priority);

/**
 * make xlog priority to syslog priority
 *
 * @param xlog_priority int priority
 * @returns syslog priority
 */
int xlog_priority_to_syslog(int xlog_priority);

#endif
