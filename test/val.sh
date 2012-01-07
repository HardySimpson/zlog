unset ZLOG_ERROR_LOG
unset ZLOG_DEBUG_LOG

#export ZLOG_DEBUG_LOG=err.log
#export ZLOG_ERROR_LOG=err.log

make -f makefile.linux clean
make -f makefile.linux

>press.log

valgrind --tool=callgrind ./test_press 1 10000
