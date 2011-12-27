#ifndef __zc_xplatform_h
#define __zc_xplatform_h

#include <limits.h>

#ifdef PATH_MAX
#define MAXLEN_PATH PATH_MAX
#else
#define MAXLEN_PATH 1024
#endif

#define MAXLEN_CFG_LINE MAXLEN_PATH*4

#ifdef NAME_MAX
#define MAXLEN_FILE NAME_MAX
#else
#define MAXLEN_FILE 255
#endif

#ifdef _WIN32
#define FILE_NEWLINE "\r\n"
#define FILE_NEWLINE_LEN 2
#else
#define FILE_NEWLINE "\n"
#define FILE_NEWLINE_LEN 1
#endif

#include <string.h>

#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )

#ifdef _WIN32
#define STRICMP(_a_,_C_,_b_) ( stricmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strnicmp(_a_,_b_,_n_) _C_ 0 )
#else
#define STRICMP(_a_,_C_,_b_) ( strcasecmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strncasecmp(_a_,_b_,_n_) _C_ 0 )
#endif

#if 0

#ifdef _WIN32
#define FILE_SEP '\\'
#else
#define FILE_SEP '/'
#endif


#include <sys/time.h>

#ifdef _WIN32
#define SD_GETTIMEOFDAY(a,b) zc_gettimeofday(a,b)
extern int zc_gettimeofday(LPFILETIME lpft, void *tzp);
#else
#define SD_GETTIMEOFDAY(a,b) gettimeofday(a,b)
extern int zc_gettimeofday(struct timeval *tp, void *tzp);
#endif

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#else
#include <time.h>
#include <io.h>			/* needed for _access  */
#include <windows.h>
#include <winsock.h>
#include <process.h>
#endif

#ifdef HAVE_STDINT_H
#       include <stdint.h>
#define  XP_UINT64 uint64_t
#define  XP_INT64 int64_t
#else
#ifndef _WIN32
#define  XP_UINT64 unsigned long long
#define  XP_INT64 long long
#else
#define  XP_UINT64 DWORD64
#define  XP_INT64 __int64
#endif
#endif

#include "zc_defs.h"

#ifdef _WIN32
#define SD_ACCESS_READ(a) _access(a,04)
#else
#define SD_ACCESS_READ(a) access(a,R_OK)
#endif

int zc_stat_ctime(const char *path, time_t * time);
#define SD_STAT_CTIME(path, time) zc_stat_ctime(path, time)

#ifndef _WIN32
#define DIFF_CMD  "/usr/bin/diff -q"
#else
#define DIFF_CMD  "comp.exe"
#endif

#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define alloca _alloca
#define strncasecmp strnicmp
#define strcasecmp stricmp
#define YY_NO_UNISTD_H
#define sleep(x) Sleep(x*1000)
#endif

/* Maybe should be using this for to mean
* MS compiler #if defined(_MSC_VER) 
*/
#ifdef _WIN32
#define pthread_t HANDLE
#define pthread_mutex_t HANDLE
#define pthread_attr_t DWORD
#define THREAD_FUNCTION DWORD (WINAPI *)(void *)

/*
* This one not obvious: you would have naturally thought of mapping to
* CreateThread()--turns out that to be safe using CRT functions
* you need to use _begintheadex().  
* cf. http://mzcn2.microsoft.com/en-us/library/7t9ha0zh.aspx
*  http://groups.google.com/group/comp.os.ms-windows.programmer.win32/browse_thread/thread/86d8624e7ee38c5d/f947ac76cd10f397?lnk=st&q=when+to+use+_beginthreadex&rnum=1#f947ac76cd10f397
* 
*/
#define pthread_create(thhandle,attr,thfunc,tharg) \
  (int)((*thhandle=(HANDLE)_beginthreadex(NULL,0,(THREAD_FUNCTION)thfunc,tharg,0,NULL))==NULL)
#define pthread_join(thread, result) \
  ((WaitForSingleObject((thread),INFINITE)!=WAIT_OBJECT_0) || !CloseHandle(thread))
#define pthread_exit() _endthreadex(0)
#define pthread_cancel(thread) TerminateThread(thread,0)

#define pthread_mutex_init(pobject,pattr) (*pobject=CreateMutex(NULL,FALSE,NULL))
#define pthread_mutex_lock(pobject) WaitForSingleObject(*pobject,INFINITE)
#define pthread_mutex_unlock(pobject) ReleaseMutex(*pobject)

#define pthread_mutex_destroy(pobject) CloseHandle(*pobject)

#endif

#endif

#endif
