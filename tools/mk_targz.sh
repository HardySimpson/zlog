#!/bin/sh

if [ "$1" = "" ]
then
    echo "Usage: mk_targz.sh <git tag only>"
    echo "Example: mktarball.sh 2.2-rc4"
    exit 1
fi

HASH=`git show-ref --hash=8 refs/tags/${1}`
PREFIX="zlog-${1}-${HASH}/"
TARBALL="/tmp/zlog-${1}-${HASH}.tar.gz"
git archive --format=tar -v --prefix=$PREFIX $1 | gzip -c > $TARBALL
cp ${TARBALL} /tmp/zlog-latest-stable.tar.gz
#git archive --format=tar -v --prefix=$PREFIX $1 -o $TARBALL
echo "File created: $TARBALL"
