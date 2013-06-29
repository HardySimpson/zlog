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

#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "zc_defs.h"

size_t zc_strtoz(const char *p, int *err)
{
	const char *u;
	char buf[128];
	long mul; /* unit multiplier */
	size_t val;
	unsigned int digits;
	
	if (err) *err = 0;
	
	/* Search the first non digit character. */
	while(isspace(*p)) p++;
	u = p;

	if (*u == '-') u++;
	while(*u && isdigit(*u)) u++;
	if (*u == '\0' || STRICMP(u, ==, "b")) {
		mul = 1;
	} else if (STRICMP(u, ==, "k")) {
		mul = 1000;
	} else if (STRICMP(u, ==, "kb")) {
		mul = 1024;
	} else if (STRICMP(u, ==, "m")) {
		mul = 1000*1000;
	} else if (STRICMP(u, ==, "mb")) {
		mul = 1024*1024;
	} else if (STRICMP(u, ==, "g")) {
		mul = 1000L*1000*1000;
	} else if (STRICMP(u, ==, "gb")) {
		mul = 1024L*1024*1024;
	} else {
		if (err) *err = 1;
		mul = 1;
	}
	digits = u-p;
		if (digits >= sizeof(buf)) {
		if (err) *err = 1;
		return 0;
	}
	memcpy(buf,p,digits);
	buf[digits] = '\0';
	val = strtol(buf,NULL,10);
	return val*mul;
}
