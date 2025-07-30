#ifndef __zlog_fmacro_h
#define __zlog_fmacro_h

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#if defined(__linux__) || defined(__OpenBSD__) || defined(_AIX)
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#else
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#endif

#ifdef _WIN32
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#endif
