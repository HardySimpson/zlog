#!/bin/sh

if [ "$1" = "" ]
then
    echo "Usage: mk_targz.sh <git tag only>"
    echo "Example: mk_targz.sh 1.2.7"
    exit 1
fi

if [ ! -d .git ]
then
    echo "Must run at git home directory"
    exit 2
fi

HASH=`git show-ref --hash=8 refs/tags/${1}`
PREFIX="zlog-${1}-${HASH}/"
TARBALL="/tmp/zlog-${1}-${HASH}.tar.gz"
git archive --format=tar -v --prefix=$PREFIX $1 | gzip -c > $TARBALL
cp ${TARBALL} /tmp/zlog-latest-stable.tar.gz
echo "File created: $TARBALL"
