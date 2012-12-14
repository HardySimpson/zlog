#!/bin/sh
#

function good_or_bad {
if [ $? -eq 0 ]; then
	echo OK
else
	echo FAILED
fi
}

function rename_makefile {
	if [ -f $1 ]; then
		mv  $1 $1.old_static
	else
		echo
		echo "Already renamed?"
	fi
}

echo -n "Finding version and setting it up for autoconf.... "
VERSION_STRING=$(sed -e 's/.*ZLOG_VERSION.*"\(.*\)"/\1/g' src/version.h)
sed -i "s/\[VERSION\]/[${VERSION_STRING}]/g" configure.ac
good_or_bad

echo -n "Renaming original static makefiles.... "
rename_makefile test/makefile && \
rename_makefile src/makefile && \
rename_makefile doc/makefile && \
rename_makefile makefile
good_or_bad


echo -n "Running aclocal ..... "
aclocal
good_or_bad

echo -n "Run libtoolize ..... "
libtoolize -fci
good_or_bad

echo -n "Running autoconf ..... "
autoconf
good_or_bad

echo -n "Running automake ..... "
automake -afi
good_or_bad

echo "Now run ./configure <parameters> to configure application"
