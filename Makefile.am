SUBDIRS =		\
	libzlog		\
	tools

if ENABLE_TEST
SUBDIRS += test
endif

if ENABLE_DOC
SUBDIRS += doc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = zlog.pc

EXTRA_DIST =	\
	autogen.sh	\
	zlog.pc.in
