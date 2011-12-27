#ifndef __xlog_mdc_h
#define __xlog_mdc_h

/**
 * @file mdc.h
 * @brief xlog mapped diagnostic contexts
 */

#include "zc_defs.h"

typedef struct {
	zc_hashtable_t *tab;
} xlog_mdc_t;

/**
 * constructor
 */
xlog_mdc_t *xlog_mdc_new(void);

/**
 * destructor
 */
void xlog_mdc_del(xlog_mdc_t *a_mdc);

/**
 * remove all values from mdc, remain map itself
 */
void xlog_mdc_clean(xlog_mdc_t *a_mdc);

/**
 * put key-value into a_mdc
 */
int xlog_mdc_put(xlog_mdc_t *a_mdc, char *key, char *value);

/**
 * get value from a_mdc
 */
char *xlog_mdc_get(xlog_mdc_t *a_mdc, char *key);

/**
 * remove key-value from a_mdc
 */
void xlog_mdc_remove(xlog_mdc_t *a_mdc, char *key);

typedef struct xlog_mdc_kv_t {
	char key[MAXLEN_PATH + 1];
	char value[MAXLEN_PATH + 1];
	size_t value_len;
} xlog_mdc_kv_t;
/**
 * get xlog_mdc_kv_t entry from a_mdc
 */
xlog_mdc_kv_t *xlog_mdc_get_kv(xlog_mdc_t *a_mdc, char *key);

#endif
