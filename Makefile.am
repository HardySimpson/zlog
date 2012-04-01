SUBDIRS =		\
	libzlog		\
	tools

if ENABLE_TEST
SUBDIRS += test
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = zlog.pc

EXTRA_DIST =	\
	autogen.sh	\
	zlog.pc.in	\
	doc/UsersGuide-CN.pdf	\
	doc/UsersGuide-EN.pdf	\
	doc/GettingStart-CN.txt	\
	doc/GettingStart-EN.txt	
