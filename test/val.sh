unset ZLOG_PROFILE_ERROR
unset ZLOG_PROFILE_DEBUG_LOG 

#export ZLOG_PROFILE_DEBUG=err.log
#export ZLOG_PROFILE_ERROR=err.log

make -f makefile.linux clean
make -f makefile.linux

rm -f press*log

valgrind --tool=callgrind ./test_press_zlog 1 10 10000
