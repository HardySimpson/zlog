#ifndef __xlog_thread_h
#define  __xlog_thread_h

/**
 * @file thread.h
 * @brief xlog_thread_t contains event, message buffer, path buffer for thread use 
 *  every application thread has one xlog_thread_t, to avoid thread competition
 */

#include "zc_defs.h"

/**
 * xlog tmap struct
 */
typedef struct {
	zc_hashtable_t *tab; /**< hashtable */
} xlog_tmap_t;

/**
 * Initer
 *
 * xlog_tmap_init() will initialise hashtable.
 * @param a_tmap the address pointed to by xlog_tmap_t.
 * @return 0 for success, -1 for fail
 */
int xlog_tmap_init(xlog_tmap_t * a_tmap);

/**
 * Update all threads in a_tmap
 *
 * xlog_tmap_update() make all threads update their buf according to
 * buf_size_min and buf_size_max

 * @param a_tmap the address pointed to by xlog_tmap_t.
 * @param buf_size_min min size of buffer in every thread 
 * @param buf_size_max min size of buffer in every thread 
 * @return 0 for success, -1 for fail
 */
int xlog_tmap_update(xlog_tmap_t *a_tmap, size_t buf_size_min, size_t buf_size_max);

/**
 * Finisher
 *
 * xlog_tmap_fini() will del all threads and destroy hashtable.
 * @param a_tmap the address pointed to by xlog_tmap_t.
 */
void xlog_tmap_fini(xlog_tmap_t * a_tmap);

/* a tmap is consist of many threads */
#include "event.h"
#include "buf.h"
#include "mdc.h"

/**
 * thread possess infomation
 */
typedef struct {
	xlog_mdc_t *mdc;
	xlog_event_t *event;		/**< all infomation from one log action */
	xlog_buf_t *pre_path_buf;	/**< pre path buffer, before %2.2s */
	xlog_buf_t *path_buf;		/**< path buffer, for dynamic file path */
	xlog_buf_t *pre_msg_buf;	/**< pre msg buffer, before %2.2s */
	xlog_buf_t *msg_buf;		/**< msg buffer, after %2.2s */
} xlog_thread_t;

/**
 * Get thread from tmap
 *
 * @param a_tmap the address pointed to by xlog_tmap_t.
 * @returns xlog_thread_t pointer, NULL for not found
 */
xlog_thread_t * xlog_tmap_get_thread(xlog_tmap_t * a_tmap);


/**
 * Create a thread from tmap
 *
 * @param a_tmap the address pointed to by xlog_tmap_t.
 * @param buf_size_min all buffers in thread's min buf size
 * @param buf_size_max all buffers in thread's max buf size
 * @returns xlog_thread_t pointer, NULL for not found
 */
xlog_thread_t * xlog_tmap_new_thread(xlog_tmap_t * a_tmap, size_t buf_size_min, size_t buf_size_max);

/**
 * Output detail of xlog_tmap_t to XLOG_ERROR_LOG.
 *
 * @param a_tmap the address pointed to by xlog_tmap_t.
 */
void xlog_tmap_profile(xlog_tmap_t * a_tmap);

#endif
