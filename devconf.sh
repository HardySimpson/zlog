#! /bin/sh

# from configure,Makefile.am->Makefile, developer use

CFLAGS="-Wall -Werror -g -O1" ./configure --prefix=/opt/develop/ --enable-test
