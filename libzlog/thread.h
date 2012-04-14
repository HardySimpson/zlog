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

#include "zc_defs.h"

typedef struct {
	zc_hashtable_t *tab;
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
	zlog_mdc_t *mdc;
	zlog_event_t *event;
	zlog_buf_t *pre_path_buf;
	zlog_buf_t *path_buf;
	zlog_buf_t *pre_msg_buf;
	zlog_buf_t *msg_buf;
} zlog_thread_t;

zlog_thread_t *zlog_tmap_get_thread(zlog_tmap_t * a_tmap);
zlog_thread_t *zlog_tmap_new_thread(zlog_tmap_t * a_tmap, size_t buf_size_min,
				    size_t buf_size_max);
void zlog_tmap_profile(zlog_tmap_t * a_tmap);

#endif
