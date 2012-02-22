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

#ifndef __zlog_priority_h
#define __zlog_priority_h

/**
 * @file priority.h
 * @brief change priority between string, int or syslog int
 */

#include "zc_defs.h"

typedef struct {
	char str[PATH_MAX];
	size_t str_len;
	int syslog_priority;
} zlog_priority_t;

/* rule init use, slow */
/* if not found, return -1 */
int zlog_priority_atoi(char *str);

/* spec ouput use, fast */
/* rule output use, fast */
/* if not found, return zlog_env_priority[254] */
zlog_priority_t *zlog_priority_get(int p);

/* conf int use, slow */
/* if p is wrong or str=="", return -1 */
int zlog_priority_set(char *str, int p, int sp);


#endif
