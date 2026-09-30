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

#ifndef _WIN32
/* Refuse a lock file that turns out to be a symbolic link, so that a name
 * planted at a shared path -- /tmp/zlog.lock is the default when there is no
 * configuration file to lock against -- cannot aim the lock somewhere else.
 * Not every platform has it; where it is missing the open behaves as before. */
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

/* 0644 rather than 0777. Nothing is ever written to this file: it exists to be
 * fcntl locked, and locking it needs write permission, which is the owner's.
 * The old mode asked for the execute bits and for anyone at all to be able to
 * take the lock; with the usual umask of 022 it did not even grant the group
 * write access it appeared to, so cross-user rotation needed the file created
 * and chowned by hand either way (see doc/UsersGuide-EN.md). */
#define ZLOG_LOCK_FILE_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#endif

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
    LOCK_FD fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW, ZLOG_LOCK_FILE_PERMS);
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
