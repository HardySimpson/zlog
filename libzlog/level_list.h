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

#ifndef __zlog_level_list_h
#define __zlog_level_list_h

#include "zc_defs.h"
#include "level.h"

zc_arraylist_t *zlog_level_list_new(void);
void zlog_level_list_del(zc_arraylist_t *levels);
void zlog_level_list_profile(zc_arraylist_t *levels, int flag);

/* conf init use, slow */
/* if l is wrong or str=="", return -1 */
int zlog_level_list_set(zc_arraylist_t *levels, char *line);

/* spec ouput use, fast */
/* rule output use, fast */
/* if not found, return levels[254] */
zlog_level_t *zlog_level_list_get(zc_arraylist_t *levels, int l);

/* rule init use, slow */
/* if not found, return -1 */
int zlog_level_list_atoi(zc_arraylist_t *levels, char *str);


#endif
