#ifndef __xlog_rule_h
#define __xlog_rule_h

#include <stdio.h>
#include "zc_defs.h"
#include "format.h"
#include "thread.h"

typedef struct xlog_rule_t xlog_rule_t;
typedef int (*xlog_rule_output_fn) (xlog_rule_t * a_rule, xlog_thread_t * a_thread);
struct xlog_rule_t {
	char category[MAXLEN_CFG_LINE + 1];
	char compare_char;
	/* [*] log any priority 
	 * [.] log priority >= rule priority, default
	 * [=] log priority == rule priority 
	 * [!] log priority != rule priority
	 */
	int priority;

	char file_path[MAXLEN_PATH + 1];
	FILE *static_file_stream;
	zc_arraylist_t *dynamic_file_specs;

	long file_maxsize;
	int syslog_facility;

	xlog_rule_output_fn output_fn;

	xlog_format_t *format;
};

xlog_rule_t *xlog_rule_new(zc_arraylist_t *formats, char *line, long line_len);
void xlog_rule_del(xlog_rule_t * a_rule);

int xlog_rule_output(xlog_rule_t * a_rule,  xlog_thread_t * a_thread);

int xlog_rule_match_category(xlog_rule_t * a_rule, char *category);

void xlog_rule_profile(xlog_rule_t * a_rule);

#endif
