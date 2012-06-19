#! /bin/sh

# from configure,Makefile.am->Makefile, developer use

CFLAGS="-Wall -Werror -g -O2 -std=c99" ./configure --prefix=/opt/develop/ --enable-test --enable-doc
