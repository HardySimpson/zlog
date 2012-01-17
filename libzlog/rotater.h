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

#ifndef __zlog_rotater_h
#define __zlog_rotater_h

/**
 * @file rotater.h
 * @brief use posix record lock between multi-processes
    and pthread_mutex lock between multi-threads for safely rotate log file
 */

#include "zc_defs.h"

typedef struct {
	pthread_mutex_t mlock;	/**< thread lock */
	int lock_fd;		/**< process lock */
} zlog_rotater_t;

extern zlog_rotater_t zlog_env_rotater;

/**
 * Rotater initer
 *
 * @param a_rot zlog_rotater pointer
 * @param lock_file global lock file for rotate, zlog will create the file,
    make sure your program has permission to create and read-write the file.
    Besides, if programs run by different users who need to write and rotater the same log file,
    make sure that each program has permission to create and read-write the same lock file.
 * @returns 0 for success, -1 for fail
 */
int zlog_rotater_init(zlog_rotater_t * a_rot, char *lock_file);

/**
 * Rotater updater
 *
 * @param a_rot zlog_rotater pointer
 * @param lock_file the new lock_file, the previous lock file will be closed and open the new one
 * @returns 0 for success, -1 for fail
 */
int zlog_rotater_update(zlog_rotater_t * a_rot, char *lock_file);

/**
 * Rotater finisher
 *
 * @param a_rot zlog_rotater pointer
 */
void zlog_rotater_fini(zlog_rotater_t * a_rot);

/**
 * the rotate fuction
 *
 * 1st, if msg_len > file_maxsize, means a single message is larger than file maxsize, return 0.
 * 2nd, if size of file_path < file_maxsize, need not rotate, return 0.
 * 3rd, size of file_path > file_maxsize, lock both pthread_mutex and record lock,
 * rotate all file start with file_path, unlock.
 * For example, file_path = "aa", files are aa, aa.1, aa.2,
 * after rotate, aa.2->aa.3, aa.1->aa.2, aa->aa.1
 *
 * @param a_rot zlog_rotater pointer
 * @param file_path the 1st file of all files that need to be rotate
 * @param file_maxsize the max size of one file
 * @param msg_len length of one message
 * @returns If msg_len>=file_maxsize, return 0.
   If size of file_path < file_maxsize, return 0.
   If rotate success, return 0.
   If rotate fail, return -1.
 */
int zlog_rotater_rotate(zlog_rotater_t * a_rot, char *file_path,
			long file_maxsize, size_t msg_len);

#endif
