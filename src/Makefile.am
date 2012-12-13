NAME=zlog

lib_LTLIBRARIES = libzlog.la

libzlog_la_CFLAGS = -I.
libzlog_la_SOURCES = buf.c buf.h \
	category.c category.h \
	category_table.c category_table.h \
	conf.c conf.h \
	event.c event.h \
	fmacros.h version.h \
	format.c format.h \
	level.c level.h \
	level_list.c level_list.h \
	mdc.c mdc.h \
	record.c record.h \
	record_table.c record_table.h \
	rotater.c rotater.h \
	rule.c rule.h \
	spec.c spec.h \
	thread.c thread.h \
	zc_arraylist.c zc_arraylist.h \
	zc_hashtable.c zc_hashtable.h \
	zc_profile.c zc_profile.h \
	zc_util.c zc_util.h \
	zc_xplatform.h \
	zc_defs.h \
	zlog.c zlog.h

include_HEADERS = zlog.h

bin_PROGRAMS = zlog-chk-conf

zlog_chk_conf_CFLAGS = -I.
zlog_chk_conf_LDADD = -lzlog
