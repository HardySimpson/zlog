#bin_dir=""
#conf_dir=""
asan_pre=""
valgrind_cmd=""

perf_wrap()
{
    rm -f $2
    sudo perf record -F 999 -g -- $1
    sudo chmod 777 perf.data
    perf script | /local/mnt/workspace/project/FlameGraph/stackcollapse-perf.pl | \
        /local/mnt/workspace/project/FlameGraph/flamegraph.pl > $2
    sudo chmod 777 press.log*
}

test_press_perf()
{
    cons_press="rm -f press.log*; $bin_dir/test_consumer_reload_press_zlog 0 200 50 $conf_dir/test_consumer_press_zlog.conf"
    norm_pess="rm -f press.log*; $bin_dir/test_consumer_reload_press_zlog 0 200 50 $conf_dir/test_press_zlog.conf"

    cd build/bin
    perf_wrap "$cons_press" profile_cons.svg
    perf_wrap "$norm_pess" profile.svg
    cd -
}

consumer_static_file_single()
{
    eval "$valgrind_cmd $asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 100 --threadN=10"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test ${FUNCNAME[0]}"
        return 1
    fi
}

# varify_static_file_single - check if normal mode and consumer mode outputs identical
varify_static_file_single()
{
    rm -f zlogA.txt
    rm -f zlogB.txt
    conf="static_file_single_A.conf"
    eval "$valgrind_cmd $asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/$conf -n 1000"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test $conf"
        return 1
    fi

    conf="consumer_static_file_single_B.conf"
    eval "$valgrind_cmd $asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/$conf -n 1000"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test $conf"
        return 1
    fi

    md5A=$(md5sum zlogA.txt | awk '{ print $1 }')
    md5B=$(md5sum zlogB.txt | awk '{ print $1 }')
    if [ "$md5A" = "$md5B" ]; then
        echo "match, $md5A == $md5B"
        return 0
    fi
    echo "failed, $md5A != $md5B"
    return 1
}

test_multi_thread()
{
    eval "$asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 10 -m 10 --threadN=10"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test ${FUNCNAME[0]}"
        return 1
    fi
}

test_multi_thread_record()
{
    eval "$asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 10 -m 10 --threadN=10 -r > output"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test ${FUNCNAME[0]}"
        return 1
    fi
}

test_multi_thread_reload()
{
    cmd="$valgrind_cmd $asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 500 -m 10 --threadN=10 --reloadcnt=8 --reloadms=400 \
        -l $conf_dir/test_consumer_static_file_single.conf \
        -l $conf_dir/test_consumer_static_file_single.conf \
        -l $conf_dir/test_static_file_single.conf \
        -l $conf_dir/test_dynamic_file.conf"
    echo "run cmd:"
    echo $cmd
    eval $cmd
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test ${FUNCNAME[0]}"
        return 1
    fi
}

test_multi_thread_recordms()
{
    eval "$asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 2 --threadN=10 -r --recordms=100 > output"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test ${FUNCNAME[0]}"
        return 1
    fi
}

test_simple()
{
    eval "$asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 10"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test ${FUNCNAME[0]}"
        return 1
    fi
}

fifo()
{
    output="output"
    target="cf64f2750cd39abc18d86cb152d0ec77"
    testcnt=350000
    eval "$asan_pre $bin_dir/fifo_test -s 0x800000 -e 16 -n $testcnt > $output"
    ret=$?
    if [[ "$ret" -ne 0 ]]; then
        echo "failed to test ${FUNCNAME[0]}"
        return 1
    fi
    # rm -f $target ; touch $target
    # i=0
    # while [ "$i" -lt "$testcnt" ]; do
        # echo "data $i" >> $target
        # i=$(($i + 1))
    # done
    md5output=$(md5sum $output | awk '{ print $1 }')
    if [ "$md5output" = "$target" ]; then
        echo "match, $md5output == $target"
        return 0
    fi
    echo "failed, $md5output != $target"
    return 1
}

while getopts "t:a::v" opt; do
  case $opt in
    t)
      testname="$OPTARG"
      ;;
    v)
      valgrind_cmd="valgrind --track-fds=yes"
      ;;
    a)
      if [[ -z "${OPTARG}" ]]; then
          asan_pre="LD_PRELOAD=/usr/lib/gcc/x86_64-linux-gnu/11/libasan.so ASAN_OPTIONS=detect_leaks=1"
      else
          asan_pre="LD_PRELOAD=$OPTARG ASAN_OPTIONS=detect_leaks=1"
      fi
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

# cd build/bin
$testname
exit $?
# cd - > /dev/null
