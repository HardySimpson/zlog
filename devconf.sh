#! /bin/sh

# from configure,Makefile.am->Makefile, developer use

CFLAGS="-Wall -Werror -g -O0 -std=c99" ./configure --prefix=/opt/develop/ --enable-test
