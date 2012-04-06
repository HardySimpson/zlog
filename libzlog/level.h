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

/**
 * @file level.h
 * @brief change level between string, int or syslog int
 */

#include "zc_defs.h"

typedef struct {
	int int_level;
	char str_capital[PATH_MAX + 1];
	char str_lowercase[PATH_MAX + 1];
	size_t str_len;
	int syslog_level;
} zlog_level_t;

int zlog_levels_init(void);
void zlog_levels_fini(void);

/* if fail, keep old not change */
int zlog_levels_reset(void);
void zlog_levels_profile(void);

/* rule init use, slow */
/* if not found, return -1 */
int zlog_level_atoi(char *str);

/* spec ouput use, fast */
/* rule output use, fast */
/* if not found, return zlog_env_level[254] */
zlog_level_t *zlog_level_get(int l);

/* conf init & update use, slow */
/* if l is wrong or str=="", return -1 */
int zlog_level_set(char *line);


#endif
