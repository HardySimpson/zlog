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

#ifndef __zlog_thread_h
#define  __zlog_thread_h

/**
 * @file thread.h
 * @brief zlog_thread_t contains event, message buffer, path buffer for thread use 
 *  every application thread has one zlog_thread_t, to avoid thread competition
 */

#include "zc_defs.h"

/**
 * zlog tmap struct
 */
typedef struct {
	zc_hashtable_t *tab; /**< hashtable */
} zlog_tmap_t;

/**
 * Initer
 *
 * zlog_tmap_init() will initialise hashtable.
 * @param a_tmap the address pointed to by zlog_tmap_t.
 * @return 0 for success, -1 for fail
 */
int zlog_tmap_init(zlog_tmap_t * a_tmap);

/**
 * Update all threads in a_tmap
 *
 * zlog_tmap_update() make all threads update their buf according to
 * buf_size_min and buf_size_max

 * @param a_tmap the address pointed to by zlog_tmap_t.
 * @param buf_size_min min size of buffer in every thread 
 * @param buf_size_max min size of buffer in every thread 
 * @return 0 for success, -1 for fail
 */
int zlog_tmap_update(zlog_tmap_t *a_tmap, size_t buf_size_min, size_t buf_size_max);

/**
 * Finisher
 *
 * zlog_tmap_fini() will del all threads and destroy hashtable.
 * @param a_tmap the address pointed to by zlog_tmap_t.
 */
void zlog_tmap_fini(zlog_tmap_t * a_tmap);

/* a tmap is consist of many threads */
#include "event.h"
#include "buf.h"
#include "mdc.h"

/**
 * thread possess infomation
 */
typedef struct {
	zlog_mdc_t *mdc;
	zlog_event_t *event;		/**< all infomation from one log action */
	zlog_buf_t *pre_path_buf;	/**< pre path buffer, before %2.2s */
	zlog_buf_t *path_buf;		/**< path buffer, for dynamic file path */
	zlog_buf_t *pre_msg_buf;	/**< pre msg buffer, before %2.2s */
	zlog_buf_t *msg_buf;		/**< msg buffer, after %2.2s */
} zlog_thread_t;

/**
 * Get thread from tmap
 *
 * @param a_tmap the address pointed to by zlog_tmap_t.
 * @returns zlog_thread_t pointer, NULL for not found
 */
zlog_thread_t * zlog_tmap_get_thread(zlog_tmap_t * a_tmap);


/**
 * Create a thread from tmap
 *
 * @param a_tmap the address pointed to by zlog_tmap_t.
 * @param buf_size_min all buffers in thread's min buf size
 * @param buf_size_max all buffers in thread's max buf size
 * @returns zlog_thread_t pointer, NULL for not found
 */
zlog_thread_t * zlog_tmap_new_thread(zlog_tmap_t * a_tmap, size_t buf_size_min, size_t buf_size_max);

/**
 * Output detail of zlog_tmap_t to ZLOG_ERROR_LOG.
 *
 * @param a_tmap the address pointed to by zlog_tmap_t.
 */
void zlog_tmap_profile(zlog_tmap_t * a_tmap);

#endif
