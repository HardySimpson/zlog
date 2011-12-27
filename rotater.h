#ifndef __xlog_rotater_h
#define __xlog_rotater_h

/**
 * @file rotater.h
 * @brief rotater use posix record lock between multi-processes
    and pthread_mutex lock between multi-threads to safely rotate log file
 */

#include "zc_defs.h"

/**
 * xlog rotater struct
 */
typedef struct {
	pthread_mutex_t mlock; 	/**< thread lock */
	int lock_fd;		/**< process lock */
} xlog_rotater_t;

/**
 * process wide rotater
 */
extern xlog_rotater_t xlog_env_rotater;

/**
 * Rotater initer
 *
 * @param a_rot xlog_rotater pointer
 * @param lock_file global lock file for rotate, xlog will create the file,
    make sure your program has permission to create and read-write the file.
    Besides, if programs run by different users who need to write and rotater the same log file,
    make sure that each program has permission to create and read-write the same lock file.
 * @returns 0 for success, -1 for fail
 */
int xlog_rotater_init(xlog_rotater_t * a_rot, char *lock_file);

/**
 * Rotater updater
 *
 * @param a_rot xlog_rotater pointer
 * @param lock_file the new lock_file, the previous lock file will be closed and open the new one
 * @returns 0 for success, -1 for fail
 */
int xlog_rotater_update(xlog_rotater_t * a_rot, char *lock_file);

/**
 * Rotater finisher
 *
 * @param a_rot xlog_rotater pointer
 */
void xlog_rotater_fini(xlog_rotater_t * a_rot);

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
 * @param a_rot xlog_rotater pointer
 * @param file_path the 1st file of all files that need to be rotate
 * @param file_maxsize the max size of one file
 * @param msg_len length of one message
 * @returns If msg_len>=file_maxsize, return 0.
   If size of file_path < file_maxsize, return 0.
   If rotate success, return 0.
   If rotate fail, return -1.
 */
int xlog_rotater_rotate(xlog_rotater_t * a_rot, char *file_path, long file_maxsize, size_t msg_len);

#endif
