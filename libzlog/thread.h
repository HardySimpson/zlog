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
 * @brief contains mdc, event, message buffer, path buffer for thread use 
 *
 * every application thread has one zlog_thread_t, to avoid thread competition
 */

#include "zc_defs.h"

typedef struct {
	zc_hashtable_t *tab; /**< hashtable */
} zlog_tmap_t;

int zlog_tmap_init(zlog_tmap_t * a_tmap);

int zlog_tmap_update(zlog_tmap_t * a_tmap, size_t buf_size_min,
		     size_t buf_size_max);

void zlog_tmap_fini(zlog_tmap_t * a_tmap);

/* a tmap is consist of many threads */
#include "event.h"
#include "buf.h"
#include "mdc.h"

typedef struct {
	zlog_mdc_t *mdc;		/**< string key-value map */
	zlog_event_t *event;		/**< all infomation from one log action */
	zlog_buf_t *pre_path_buf;	/**< pre path buffer, before %2.2s */
	zlog_buf_t *path_buf;		/**< path buffer, for dynamic file path */
	zlog_buf_t *pre_msg_buf;	/**< pre msg buffer, before %2.2s */
	zlog_buf_t *msg_buf;		/**< msg buffer, after %2.2s */
} zlog_thread_t;

zlog_thread_t *zlog_tmap_get_thread(zlog_tmap_t * a_tmap);

zlog_thread_t *zlog_tmap_new_thread(zlog_tmap_t * a_tmap, size_t buf_size_min,
				    size_t buf_size_max);

void zlog_tmap_profile(zlog_tmap_t * a_tmap);

#endif
