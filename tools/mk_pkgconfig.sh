#!/bin/sh
# Script to generate a pkgconfig file

if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ]
then
    echo "Usage: $0 <DESTDIR> <PREFIX> <LIBRARY_PATH> <VERSION>"
    echo "Example: $0 /usr/local/ lib 1.2.12"
    exit 1
fi

DESTDIR="$1"
PREFIX="$2"
LIBRARY_PATH="$3"
VERSION="$4"
TARGET_DIR="${DESTDIR}${PREFIX}/${LIBRARY_PATH}/pkgconfig"
TARGET="${TARGET_DIR}/zlog.pc"

mkdir -p ${TARGET_DIR}

cat << EOF > ${TARGET}
#zlog pkg-config source file

prefix=${PREFIX}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: zlog
Description: zlog C logging suite
Version: ${VERSION}
Requires:
Conflicts:
Libs: -L\${libdir} -lzlog
Libs.private:
Cflags: -I\${includedir}
EOF
