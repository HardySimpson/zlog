#! /bin/sh

# from configure,Makefile.am->Makefile, developer use

CFLAGS="-Wall -Werror" ./configure --prefix=/opt/develop/ --enable-test
