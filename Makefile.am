#AM_CPPFLAGS = -D_GNU_SOURCE
# -D_GNU_SOURCE for pthread header files
#AM_CFLAGS = -pthread

lib_LTLIBRARIES = libxlog.la
libxlog_la_SOURCES = buf.c category.c conf.c event.c format.c mdc.c priority.c thread.c rotater.c rule.c spec.c xlog.c zc_arraylist.c zc_error.c zc_hashtable.c zc_util.c zc_xplatform.c buf.h category.h conf.h event.h format.h mdc.h priority.h thread.h rotater.h rule.h spec.h zc_arraylist.h zc_defs.h zc_error.h zc_hashtable.h zc_util.h zc_xplatform.h
libxlog_la_LIBADD = -lpthread
include_HEADERS = xlog.h

bin_PROGRAMS = xlog-chk-conf
xlog_chk_conf_SOURCES = xlog-chk-conf.c
xlog_chk_conf_LDADD = libxlog.la $(PTREADLIB)
xlog_chk_conf_CPPFLAGS = -I$(srcdir)/

SUBDIRS = . test

EXTRA_DIST = autogen.sh
