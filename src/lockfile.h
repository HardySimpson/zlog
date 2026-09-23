/**
 * =============================================================================
 *
 * \file lockfile.h
 * \breif
 * \version 1.0
 * \date 2013-07-07 12:34:20
 * \author  Song min.Li (Li), lisongmin@126.com
 * \copyright Copyright (c) 2013, skybility
 *
 * =============================================================================
 */

#ifndef ZLOG_LOCKFILE_H
#define ZLOG_LOCKFILE_H

#ifdef _WIN32
#include <Windows.h>
#include <WinBase.h>
#define LOCK_FD         HANDLE
#define INVALID_LOCK_FD INVALID_HANDLE_VALUE
#else //_WIN32
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#define LOCK_FD         int
#define INVALID_LOCK_FD -1
#endif
#include <stdbool.h>

/**
 * lock file.
 */
LOCK_FD lock_file(char* path);

/**
 * unlock file.
 */
bool unlock_file(LOCK_FD fd);

#endif //ZLOG_LOCKFILE_H
