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

#ifndef __zlog_deepness_h
#define __zlog_deepness_h

typedef struct zlog_deepness_s zlog_deepness_t;

struct zlog_deepness_s {
	zc_sds sign;
	unsigned int perm;
	size_t buffer_size;
	size_t flush_size;
	size_t flush_count;
	ssize_t fsync_count;
};

zlog_deepness_t *zlog_deepness_new(char *line);
void zlog_deepness_del(zlog_deepness_t * a_deepness);
void zlog_deepness_profile(zlog_deepness_t * a_deepness, int flag);

#define zlog_deepness_has_name(a_deepness, fname) \
	STRCMP(a_deepness->name, ==, fname)

#endif
