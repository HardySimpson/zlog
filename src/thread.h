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
#include "event.h"
#include "buf.h"
#include "mdc.h"

typedef struct {
	zlog_mdc_t *mdc;
	zlog_event_t *event;

	zlog_buf_t *pre_path_buf;
	zlog_buf_t *path_buf;
	zlog_buf_t *archive_path_buf;
	zlog_buf_t *pre_msg_buf;
	zlog_buf_t *msg_buf;

	zlog_buf_t *pre_msg_buf_backup;
	zlog_buf_t *msg_buf_backup;
} zlog_thread_t;


void zlog_thread_del(zlog_thread_t * a_thread);
void zlog_thread_profile(zlog_thread_t * a_thread, int flag);
zlog_thread_t *zlog_thread_new(size_t buf_size_min, size_t buf_size_max);

int zlog_thread_update_msg_buf(zlog_thread_t * a_thread, size_t buf_size_min, size_t buf_size_max);
void zlog_thread_commit_msg_buf(zlog_thread_t * a_thread);
void zlog_thread_rollback_msg_buf(zlog_thread_t * a_thread);

#endif
