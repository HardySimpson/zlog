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

#ifndef __zlog_conf_h
#define __zlog_conf_h

#include "zc_defs.h"
#include "format.h"

typedef struct zlog_conf_s zlog_conf_t;

zlog_conf_t *zlog_conf_new(char *conf_file);
void zlog_conf_del(zlog_conf_t * a_conf);
void zlog_conf_profile(zlog_conf_t * a_conf, int flag);

zc_arraylist_t *zlog_conf_get_rules(zlog_conf_t *a_conf);
void zlog_conf_get_buf_size(zlog_conf_t *a_conf,
		size_t * buf_size_min, size_t * buf_size_max);
char *zlog_conf_get_file(zlog_conf_t *a_conf);

#endif
