SUBDIRS =		\
	libzlog		\
	zlog-chk-conf

if ENABLE_TEST
SUBDIRS += test
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = zlog.pc

EXTRA_DIST = autogen.sh zlog.pc.in
