#ifndef __xlog_category_h
#define __xlog_category_h

#include "zc_defs.h"

/**
 * @file category.h
 * @brief xlog category and cmap(global hashtable of categories)
 */

/**
 * Cats is the hashtable of category
 */
typedef struct {
	zc_hashtable_t *tab;	/**< the table */
} xlog_cmap_t;

/**
 * Cats initer
 *
 * xlog_cmap_init() will initialise hashtable.
 * @param a_cmap the address pointed to by xlog_cmap_t.
 * @return 0 for success, -1 for fail
 */
int xlog_cmap_init(xlog_cmap_t * a_cmap);

/**
 * Update all categories in a_cmap
 *
 * xlog_cmap_update() make all categories update their rules from rule list.
 * @param a_cmap the address pointed to by xlog_cmap_t.
 * @param rules list of all rules
 * @return 0 for success, -1 for fail
 */
int xlog_cmap_update(xlog_cmap_t *a_cmap, zc_arraylist_t *rules);

/**
 * Cats finisher
 *
 * xlog_cmap_fini() will del all categories and destroy hashtable.
 * @param a_cmap the address pointed to by xlog_cmap_t.
 */
void xlog_cmap_fini(xlog_cmap_t * a_cmap);

/**
 * Output detail of xlog_cmap_t to XLOG_ERROR_LOG.
 *
 * @param a_cmap the address pointed to by xlog_cmap_t.
 */
void xlog_cmap_profile(xlog_cmap_t * a_cmap);

/**
 * xlog category struct
 */
typedef struct xlog_category_t {
	char name[MAXLEN_PATH + 1];	/**< name, must consist of alpha, digits or _ */
	size_t name_len;		/**< strlen(name) */
	zc_arraylist_t *match_rules;	/**< list of rules which match this category */
} xlog_category_t;

/**
 * Get a xlog_category_t from hashtable, if not found, use rules to create one.
 *
 * @param a_cmap the address pointed to by xlog_cmap_t.
 * @param category_name name of category
 * @param rules if not found, use rules to create a new category
 * @returns poiter of xlog_category_t for success, NULL for fail
 */
xlog_category_t *xlog_cmap_fetch_category(xlog_cmap_t *a_cmap,
				char *category_name, zc_arraylist_t *rules);

#include "thread.h"

/**
 * Output one message to several rules.
 *
 * @param a_cat contains all rules that maybe use to output, up to priority
 * @param a_thread the message's raw material and formated buffer are in a_thread,
 * which belongs to one thread
 * @returns 0 for success, -1 for fail
 */
int xlog_category_output(xlog_category_t *a_cat, xlog_thread_t *a_thread);

#endif
