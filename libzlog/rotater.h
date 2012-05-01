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

#ifndef __zlog_rotater_h
#define __zlog_rotater_h

#include "zc_defs.h"

typedef struct zlog_rotater_s zlog_rotater_t;

zlog_rotater_t *zlog_rotater_new(char *lock_file);
void zlog_rotater_del(zlog_rotater_t *a_rot);
int zlog_rotater_rotate(zlog_rotater_t *a_rot,
		char *file_path, long file_max_size, int file_max_count,
		size_t msg_len);

void zlog_rotater_profile(zlog_rotater_t *a_rot, int flag);

#endif
