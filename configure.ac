#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([zlog], [1.2.7], [HardySimpson1984@gmail.com])
AC_CONFIG_SRCDIR([src/version.h])
AC_CONFIG_MACRO_DIR([m4])
# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h stdlib.h string.h strings.h sys/time.h syslog.h unistd.h])

# Checks for programs
AM_CONDITIONAL([HAVE_LYX], [test "$(which lyx)" = "true"])
AM_CONDITIONAL([HAVE_HEVEA], [test "$(which hevea)" = "true"])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit gethostname gettimeofday localtime_r memmove memset setenv strchr strrchr strtol])

AC_CONFIG_FILES([doc/Makefile
                 Makefile
                 src/Makefile
                 test/Makefile
                 tools/Makefile])
AC_OUTPUT
