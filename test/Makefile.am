AM_CPPFLAGS =					\
	-I$(top_srcdir)/libzlog

LDADD =						\
	$(top_builddir)/libzlog/libzlog.la	\
	$(PTREADLIB)

noinst_PROGRAMS =	\
	test_filemv	\
	test_hashtable	\
	test_hello	\
	test_hex	\
	test_init	\
	test_press	\
	test_printf	\
	test_syslog	\
	test_syslog_press	\
	test_conf	\
	test_mdc	\
	test_level	\
	test_default

test_conf_SOURCES = test_conf.c
test_filemv_SOURCES = test_filemv.c
test_hashtable_SOURCES = test_hashtable.c
test_hello_SOURCES = test_hello.c
test_hex_SOURCES = test_hex.c
test_init_SOURCES = test_init.c
test_press_SOURCES = test_press.c
test_printf_SOURCES = test_printf.c
test_syslog_SOURCES = test_syslog.c
test_syslog_press_SOURCES = test_syslog_press.c
test_mdc_SOURCES = test_mdc.c
test_level_SOURCES = test_level.c
test_default_SOURCES = test_default.c

EXTRA_DIST =		\
	test_conf.conf	\
	test_hello.conf	\
	test_hex.conf	\
	test_init.conf	\
	test_press.conf	\
	test_mdc.conf	\
	test_syslog.conf	\
	test_level.conf	\
	test_default.conf
