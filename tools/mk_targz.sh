#!/bin/sh

if [ "$1" = "" ]
then
    echo "Usage: mk_targz.sh <git tag, branch or commit>"
    echo "Example: mktarball.sh 2.2-rc4"
    exit 1
fi

PREFIX="zlog-${1}/"
TARBALL="/tmp/zlog-${1}.tar.gz"
git archive --format=tar -v --prefix=$PREFIX $1 | gzip -c > $TARBALL
#git archive --format=tar -v --prefix=$PREFIX $1 -o $TARBALL
echo "File created: $TARBALL"
