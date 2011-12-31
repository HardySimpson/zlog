AC_PREREQ(2.59)
AC_INIT([xlog], [0.9.0], [HardySimpson1984@gmail.com])
AM_INIT_AUTOMAKE([gnu -Wall -Werror])
AC_CONFIG_SRCDIR([libxlog/xlog.h])
AC_CONFIG_HEADER([config.h])
AC_GNU_SOURCE
#LT_INIT(disable-shared) 

# Checks for programs.
AC_PROG_CPP
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

AC_CHECK_LIB([pthread], [pthread_rwlock_wrlock], [PTREADLIB=-lpthread])
if test "x$PTREADLIB" != "x-lpthread"; then
	AC_MSG_ERROR([You need libpthread in order to compile libxlog])
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h stdlib.h string.h strings.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gethostname gettimeofday localtime_r memmove memset setenv strchr strrchr strstr strtol])

if test "x$GCC" = "xyes"; then
  CFLAGS="$CFLAGS -D_GNU_SOURCE -Wall -Werror -pthread"
fi

AC_ARG_ENABLE(test,
	[AC_HELP_STRING([--enable-test], [Compile tests])],
	enable_test=yes)

AM_CONDITIONAL(ENABLE_TEST, test x$enable_test = xyes)

AC_CONFIG_FILES([
xlog.pc
Makefile
libxlog/Makefile
xlog-chk-conf/Makefile
test/Makefile
])

AC_OUTPUT
