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

#ifndef __zlog_level_h
#define __zlog_level_h

typedef struct zlog_level_s {
	int number;
	zc_sds str_upper;
	zc_sds str_lower;
       	int syslog_level;
} zlog_level_t;

zlog_level_t *zlog_level_new(char *line);
void zlog_level_del(zlog_level_t *a_level);
void zlog_level_profile(zlog_level_t *a_level, int flag);

#endif
