#ifndef __xlog_spec_h
#define __xlog_spec_h

#include "event.h"
#include "buf.h"
#include "thread.h"

typedef struct xlog_spec_t xlog_spec_t;

typedef int (*xlog_spec_gen_fn) (xlog_spec_t * a_spec,
		xlog_thread_t * a_thread, xlog_buf_t * a_buf);

struct xlog_spec_t {
	char *str;
	int len;

	char time_fmt[MAXLEN_CFG_LINE + 1];

	int ms_count;
	size_t ms_offset[MAXLEN_CFG_LINE / 2 + 2];
	int us_count;
	size_t us_offset[MAXLEN_CFG_LINE / 2 + 2];
	size_t time_len;

	char mdc_key[MAXLEN_PATH + 1];

	char print_fmt[MAXLEN_CFG_LINE + 1];
	xlog_spec_gen_fn gen_fn;
};

int xlog_spec_gen_msg(xlog_spec_t * a_spec, xlog_thread_t *a_thread);
int xlog_spec_gen_path(xlog_spec_t * a_spec, xlog_thread_t *a_thread);

xlog_spec_t * xlog_spec_new(char *pattern_start, char **pattern_end);
void xlog_spec_del(xlog_spec_t * a_spec);

void xlog_spec_profile(xlog_spec_t * a_spec);

#endif
