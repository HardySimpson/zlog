#!/bin/sh
# Script to generate a pkgconfig file

if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ]
then
    echo "Usage: $0 <PREFIX> <LIBRARY_PATH> <VERSION>"
    echo "Example: $0 /usr/local/ lib 1.2.12"
    exit 1
fi

PREFIX="$1"
LIBRARY_PATH="$2"
VERSION="$3"
TARGET_DIR="${PREFIX}/${LIBRARY_PATH}/pkgconfig"
TARGET="${TARGET_DIR}/zlib.pc"

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
