/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson@gmail.com>
 *
 * The Xlog Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Xlog Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Xlog Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "priority.h"
#include "zc_defs.h"
#include "syslog.h"

static const char *const priorities[] = {
	"UNKOWN",
	"DEBUG",
	"INFO",
	"NOTICE",
	"WARN",
	"ERROR",
	"FATAL",
};

static size_t npriorities = sizeof(priorities) / sizeof(priorities[0]);

char *xlog_priority_itostr(int priority)
{
	if ((priority < 0) || (priority > 6))
		priority = 0;

	return (char *)priorities[priority];
}

int xlog_priority_strtoi(char *priority)
{
	int i;

	if (!priority)
		return 0;

	for (i = 0; i < npriorities; i++) {
		if (STRICMP(priority, ==, priorities[i]))
			return i;
	}

	return 0;
}

int xlog_priority_to_syslog(int xlog_priority)
{
	switch(xlog_priority) {
		case 0:
			return LOG_INFO;
			break;
		case 1:
			return LOG_DEBUG;
			break;
		case 2:
			return LOG_INFO;
			break;
		case 3:
			return LOG_NOTICE;
			break;
		case 4:
			return LOG_WARNING;
			break;
		case 5:
			return LOG_ERR;
			break;
		case 6:
			return LOG_ALERT;
			break;
		default:
			return LOG_INFO;
			break;
	}
}
