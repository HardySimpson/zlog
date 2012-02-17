bin_PROGRAMS = zlog-chk-conf zlog-gen-conf

zlog_chk_conf_SOURCES = zlog-chk-conf.c

zlog_gen_conf_SOURCES = zlog-gen-conf.c


LDADD =				\
	$(top_builddir)/libzlog/libzlog.la	\
	$(PTREADLIB)

AM_CPPFLAGS = -I$(top_srcdir)/libzlog

