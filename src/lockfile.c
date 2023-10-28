/**
 * =============================================================================
 *
 * \file lockfile.c
 * \breif
 * \version 1.0
 * \date 2013-07-07 11:55:38
 * \author  Song min.Li (Li), lisongmin@126.com
 * \copyright Copyright (c) 2013, skybility
 *
 * =============================================================================
 */

#include "lockfile.h"
#include "zc_profile.h"

LOCK_FD lock_file(char* path) {
    if (!path || strlen(path) <= 0) {
        return INVALID_LOCK_FD;
    }
#ifdef _WIN32
    LOCK_FD fd = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd == INVALID_LOCK_FD) {
        DWORD err = GetLastError();
        zc_error("lock file error : %d ", err);
    }
#else
    LOCK_FD fd = open(path, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd == INVALID_LOCK_FD) {
        zc_error("lock file error : %s ", strerror(errno));
    }
#endif
    return fd;
}

bool unlock_file(LOCK_FD fd) {
    if (fd == INVALID_LOCK_FD) {
        return true;
    }
#ifdef _WIN32
    bool ret = CloseHandle(fd);
    if (ret == false) {
        DWORD err = GetLastError();
        zc_error("unlock file error : %d ", err);
    }
#else
    bool ret = close(fd) == 0;
    if (ret == false) {
        zc_error("unlock file error : %s ", strerror(errno));
    }
#endif
    return ret;
}