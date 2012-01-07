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
 * @brief zlog priority functions
 */

/**
 * make 0-5 to "UNKNOWN", "DEBUG"...
 *
 * @param priority int priority
 * @returns const static string of priority
 */
char *zlog_priority_itostr(int priority);

/**
 * make "UNKNOWN", "DEBUG"... to 0-5
 *
 * @param string of priority
 * @returns priority int priority
 */
int zlog_priority_strtoi(char *priority);

/**
 * make zlog priority to syslog priority
 *
 * @param zlog_priority int priority
 * @returns syslog priority
 */
int zlog_priority_to_syslog(int zlog_priority);

#endif
