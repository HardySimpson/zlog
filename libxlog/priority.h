/*
 * This file is part of the Xlog Library.
 *
 * Copyright (C) 2011 by Hardy Simpson <HardySimpson1984@gmail.com>
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

#ifndef __xlog_priority_h
#define __xlog_priority_h

/**
 * @file priority.h
 * @brief xlog priority functions
 */

/**
 * make 0-5 to "UNKNOWN", "DEBUG"...
 *
 * @param priority int priority
 * @returns const static string of priority
 */
char *xlog_priority_itostr(int priority);

/**
 * make "UNKNOWN", "DEBUG"... to 0-5
 *
 * @param string of priority
 * @returns priority int priority
 */
int xlog_priority_strtoi(char *priority);

/**
 * make xlog priority to syslog priority
 *
 * @param xlog_priority int priority
 * @returns syslog priority
 */
int xlog_priority_to_syslog(int xlog_priority);

#endif
