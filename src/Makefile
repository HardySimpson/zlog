# zlog makefile
#
# Copyright (c) Hardy Simpson
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

OBJ=    \
  buf.o    \
  category.o    \
  category_table.o    \
  conf.o    \
  event.o    \
  format.o    \
  level.o    \
  level_list.o    \
  mdc.o    \
  record.o    \
  record_table.o    \
  rotater.o    \
  rule.o    \
  spec.o    \
  thread.o    \
  zc_arraylist.o    \
  zc_hashtable.o    \
  zc_profile.o    \
  zc_util.o    \
  lockfile.o \
  zlog.o
BINS=zlog-chk-conf
LIBNAME=libzlog

ZLOG_MAJOR=1
ZLOG_MINOR=2
ZLOG_PATCH=12

ZLOG_CSTRING_VERSION := $(shell printf '\\"%d.%d.%d\\"' $(ZLOG_MAJOR) $(ZLOG_MINOR) $(ZLOG_PATCH))

# for mingw (cut the last part of the uname)
UNAME := $(shell uname | cut -c -5)

ifeq ($(UNAME), MINGW)
   $(print compiling on mingw !!)
  OBJ := $(OBJ) zlog_win.o
endif

# Fallback to gcc when $CC is not in $PATH.
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')
OPTIMIZATION?=-O2

ifeq ($(UNAME), MINGW)
#XXX: ignore warnings on mingw for now
WARNINGS=-Wall -Wstrict-prototypes -fwrapv
else
WARNINGS=-Wall -Werror -Wstrict-prototypes -fwrapv
endif

DEBUG?= -g -ggdb
REAL_CFLAGS=$(OPTIMIZATION) -fPIC -pthread $(CFLAGS) $(WARNINGS) $(DEBUG)
REAL_LDFLAGS=$(LDFLAGS) -pthread

DYLIBSUFFIX=so
STLIBSUFFIX=a
DYLIB_MINOR_NAME=$(LIBNAME).$(DYLIBSUFFIX).$(ZLOG_MAJOR).$(ZLOG_MINOR)
DYLIB_MAJOR_NAME=$(LIBNAME).$(DYLIBSUFFIX).$(ZLOG_MAJOR)
DYLIBNAME=$(LIBNAME).$(DYLIBSUFFIX)
DYLIB_MAKE_CMD=$(CC) -shared -Wl,-soname,$(DYLIB_MINOR_NAME) -o $(DYLIBNAME) $(LDFLAGS)
STLIBNAME=$(LIBNAME).$(STLIBSUFFIX)
STLIB_MAKE_CMD=ar rcs $(STLIBNAME)

# Installation related variables
PREFIX?=/usr/local
INCLUDE_PATH?=include
LIBRARY_PATH?=lib
BINARY_PATH=bin
PKGCONFIG_PATH?=pkgconfig
INSTALL_INCLUDE_PATH= $(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH= $(PREFIX)/$(LIBRARY_PATH)
INSTALL_BINARY_PATH=  $(PREFIX)/$(BINARY_PATH)
INSTALL_PKGCONFIG_PATH= $(PREFIX)/$(LIBRARY_PATH)/$(PKGCONFIG_PATH)

# Platform-specific overrides
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
compiler_platform := $(shell sh -c '$(CC) --version|grep -i apple')

ifeq ($(uname_S),SunOS)
#  REAL_LDFLAGS+= -ldl -lnsl -lsocket
  DYLIB_MAKE_CMD=$(CC) -G -o $(DYLIBNAME) -h $(DYLIB_MINOR_NAME) $(LDFLAGS)
  INSTALL= cp -r
endif

# For Darwin builds, check the compiler platform above is not empty. The covers cross compilation on Linux
ifneq ($(compiler_platform),)
  DYLIBSUFFIX=dylib
  DYLIB_MINOR_NAME=$(LIBNAME).$(ZLOG_MAJOR).$(ZLOG_MINOR).$(DYLIBSUFFIX)
  DYLIB_MAJOR_NAME=$(LIBNAME).$(ZLOG_MAJOR).$(DYLIBSUFFIX)
  DYLIB_MAKE_CMD=$(CC) -dynamiclib -install_name $(INSTALL_LIBRARY_PATH)/$(DYLIB_MINOR_NAME) -o $(DYLIBNAME) $(LDFLAGS)
  REAL_CFLAGS+= -D_DARWIN_C_SOURCE
endif

ifeq ($(uname_S),AIX)
  # this logic of minor major is not relevant on AIX or at least not widely used
  # not to mention dynamic linker .a preference...
  DYLIB_MAKE_CMD=$(CC) -shared -Wl,-G,-b64 -maix64 -pthread -o $(DYLIBNAME) $(LDFLAGS)
  REAL_CFLAGS+= -maix64
  STLIB_MAKE_CMD=OBJECT_MODE=64 ar rcs $(STLIBNAME) $(DYLIB_MAJOR_NAME)
endif

ifeq ($(UNAME), MINGW)
   $(print compiling on mingw !!)
   DYLIBSUFFIX=dll
   STLIBSUFFIX=nogo

   REAL_CFLAGS  := $(REAL_CFLAGS) -I$(INSTALL_INCLUDE_PATH)
   REAL_LDFLAGS := $(REAL_LDFLAGS) -L$(INSTALL_LIBRARY_PATH) -lws2_32 -lunixem
endif

all: $(DYLIBNAME) $(BINS)

# Deps (use make dep to generate this)
buf.o: buf.c zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h buf.h
category.o: category.c fmacros.h category.h zc_defs.h zc_profile.h \
 zc_arraylist.h zc_hashtable.h zc_xplatform.h zc_util.h thread.h event.h \
 buf.h mdc.h rule.h format.h rotater.h record.h
category_table.o: category_table.c zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h category_table.h category.h \
 thread.h event.h buf.h mdc.h
conf.o: conf.c fmacros.h conf.h zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h format.h thread.h event.h buf.h \
 mdc.h rotater.h rule.h record.h level_list.h level.h
event.o: event.c fmacros.h zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h event.h
format.o: format.c zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h thread.h event.h buf.h mdc.h spec.h format.h
level.o: level.c zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h level.h
level_list.o: level_list.c zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h level.h level_list.h
mdc.o: mdc.c mdc.h zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h
record.o: record.c zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h record.h
record_table.o: record_table.c zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h record_table.h record.h
rotater.o: rotater.c zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h rotater.h
rule.o: rule.c fmacros.h rule.h zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h format.h thread.h event.h buf.h \
 mdc.h rotater.h record.h level_list.h level.h spec.h
spec.o: spec.c fmacros.h spec.h event.h zc_defs.h zc_profile.h \
 zc_arraylist.h zc_hashtable.h zc_xplatform.h zc_util.h buf.h thread.h \
 mdc.h level_list.h level.h
thread.o: thread.c zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h event.h buf.h thread.h mdc.h
zc_arraylist.o: zc_arraylist.c zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h
zc_hashtable.o: zc_hashtable.c zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h
zc_profile.o: zc_profile.c fmacros.h zc_profile.h zc_xplatform.h
zc_util.o: zc_util.c zc_defs.h zc_profile.h zc_arraylist.h zc_hashtable.h \
 zc_xplatform.h zc_util.h
zlog-chk-conf.o: zlog-chk-conf.c fmacros.h zlog.h
lockfile.o: lockfile.c
zlog.o: zlog.c fmacros.h conf.h zc_defs.h zc_profile.h zc_arraylist.h \
 zc_hashtable.h zc_xplatform.h zc_util.h format.h thread.h event.h buf.h \
 mdc.h rotater.h category_table.h category.h record_table.h \
 record.h rule.h version.h
zlog_win.o: zlog_win.c

version.h: Makefile
	@echo "Generating version.h"
	@echo "#define ZLOG_VERSION $(ZLOG_CSTRING_VERSION)" > version.h

zlog.pc: Makefile
	@echo "Generating zlog.pc"
	@echo ""                                         >> zlog.pc
	@echo "prefix=$(PREFIX)"                         >> zlog.pc
	@echo "exec_prefix=\$${prefix}"                  >> zlog.pc
	@echo "libdir=\$${exec_prefix}/$(LIBRARY_PATH)"  >> zlog.pc
	@echo "includedir=\$${prefix}/$(INCLUDE_PATH)"   >> zlog.pc
	@echo ""                                         >> zlog.pc
	@echo "Name: zlog"                               >> zlog.pc
	@echo "Description: The zlog C logger framework" >> zlog.pc
	@echo "URL: http://hardysimpson.github.io/zlog/" >> zlog.pc
	@echo "Version: $(ZLOG_MAJOR).$(ZLOG_MINOR).$(ZLOG_PATCH)" \
	                                                 >> zlog.pc
	@echo "Libs: -L\$${libdir} -lzlog"               >> zlog.pc
	@echo "Cflags: -I\$${includedir}"                >> zlog.pc
	@echo ""                                         >> zlog.pc

$(DYLIBNAME): $(OBJ)
	$(DYLIB_MAKE_CMD) $(OBJ) $(REAL_LDFLAGS)
	# for use in test folder - linux and requirement for aix runtime
	# resolving
	cp -f $(DYLIBNAME) $(DYLIB_MAJOR_NAME)
	cp -f $(DYLIBNAME) $(DYLIB_MINOR_NAME)

$(STLIBNAME): $(OBJ)
	$(STLIB_MAKE_CMD) $(OBJ)

pkgconf: zlog.pc

dynamic: $(DYLIBNAME)
static: $(STLIBNAME)

# Binaries:
zlog-chk-conf: zlog-chk-conf.o $(STLIBNAME) $(DYLIBNAME)
	$(CC) -o $@ zlog-chk-conf.o -L. -lzlog $(REAL_LDFLAGS)

.c.o:
	$(CC) -std=c99 -pedantic -c $(REAL_CFLAGS) $<

clean:
	rm -rf $(DYLIBNAME) $(STLIBNAME) $(BINS) *.o *.gcda *.gcno *.gcov $(DYLIB_MINOR_NAME) $(DYLIB_MAJOR_NAME) zlog.pc

dep:
	$(CC) -MM *.c

# Installation target

ifeq ($(uname_S),SunOS)
  INSTALL?= cp -r
endif

ifeq ($(uname_S),AIX)
  INSTALL?= cp -r
endif


INSTALL?= cp -a

install: $(DYLIBNAME) $(STLIBNAME) zlog.pc
	mkdir -p $(INSTALL_INCLUDE_PATH) $(INSTALL_LIBRARY_PATH) $(INSTALL_BINARY_PATH) $(INSTALL_PKGCONFIG_PATH)
	$(INSTALL) zlog.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) zlog-chk-conf $(INSTALL_BINARY_PATH)
	$(INSTALL) $(DYLIBNAME) $(INSTALL_LIBRARY_PATH)/$(DYLIB_MINOR_NAME)
	cd $(INSTALL_LIBRARY_PATH) && ln -sf $(DYLIB_MINOR_NAME) $(DYLIB_MAJOR_NAME)
	cd $(INSTALL_LIBRARY_PATH) && ln -sf $(DYLIB_MAJOR_NAME) $(DYLIBNAME)
	$(INSTALL) $(STLIBNAME) $(INSTALL_LIBRARY_PATH)
	$(INSTALL) zlog.pc $(INSTALL_PKGCONFIG_PATH)

32bit:
	@echo ""
	@echo "WARNING: if this fails under Linux you probably need to install libc6-dev-i386"
	@echo ""
	$(MAKE) CFLAGS="-m32" LDFLAGS="-m32"

gprof:
	$(MAKE) CFLAGS="-pg" LDFLAGS="-pg"

gcov:
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs"

coverage: gcov
	make check
	mkdir -p tmp/lcov
	lcov -d . -c -o tmp/lcov/hiredis.info
	genhtml --legend -o tmp/lcov/report tmp/lcov/hiredis.info

noopt:
	$(MAKE) OPTIMIZATION=""

.PHONY: all clean dep install 32bit gprof gcov noopt
