unset XLOG_ERROR_LOG
unset XLOG_DEBUG_LOG

make -f makefile.linux clean
make -f makefile.linux

>press.log

valgrind --tool=callgrind ./test_press 1 10000
