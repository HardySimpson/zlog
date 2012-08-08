AM_CPPFLAGS =					\
	-I$(top_srcdir)/libzlog

LDADD =						\
	$(top_builddir)/libzlog/libzlog.la	\
	$(PTREADLIB)

noinst_PROGRAMS =	\
	test_tmp	\
	test_buf	\
	test_bitmap	\
	test_filemv	\
	test_hashtable	\
	test_hello	\
	test_hex	\
	test_init	\
	test_press_zlog		\
	test_press_zlog2	\
	test_press_syslog	\
	test_press_write	\
	test_press_write2	\
	test_syslog	\
	test_conf	\
	test_mdc	\
	test_level	\
	test_leak	\
	test_default	\
	test_record	\
	test_profile

test_tmp_SOURCES = test_tmp.c
test_buf_SOURCES = test_buf.c
test_conf_SOURCES = test_conf.c
test_filemv_SOURCES = test_filemv.c
test_hashtable_SOURCES = test_hashtable.c
test_hello_SOURCES = test_hello.c
test_hex_SOURCES = test_hex.c
test_init_SOURCES = test_init.c
test_press_zlog_SOURCES = test_press_zlog.c
test_press_zlog2_SOURCES = test_press_zlog2.c
test_press_write_SOURCES = test_press_write.c
test_press_write2_SOURCES = test_press_write2.c
test_press_syslog_SOURCES = test_press_syslog.c
test_syslog_SOURCES = test_syslog.c
test_mdc_SOURCES = test_mdc.c
test_level_SOURCES = test_level.c test_level.h
test_default_SOURCES = test_default.c
test_profile_SOURCES = test_profile.c

EXTRA_DIST =		\
	test_conf.conf	\
	test_hello.conf	\
	test_hex.conf	\
	test_init.conf	\
	test_press_zlog.conf	\
	test_press_zlog2.conf	\
	test_mdc.conf	\
	test_syslog.conf	\
	test_level.conf		\
	test_default.conf	\
	test_profile.conf

clean-local :
	-rm -f press.*
