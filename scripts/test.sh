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
}

test_multi_thread_record()
{
    eval "$asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 10 -m 10 --threadN=10 -r"
}

test_multi_thread_reload()
{
    cmd="$valgrind_cmd $asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 1000 -m 10 --threadN=10 --reloadcnt=10 --reloadms=400 \
        -l $conf_dir/test_consumer_static_file_single.conf \
        -l $conf_dir/test_consumer_static_file_single.conf \
        -l $conf_dir/test_static_file_single.conf \
        -l $conf_dir/test_dynamic_file.conf"
    echo "run cmd:"
    echo $cmd
    eval $cmd
}

test_multi_thread_recordms()
{
    eval "$asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 2 --threadN=10 -r --recordms=100"
}

test_simple()
{
    eval "$asan_pre $bin_dir/test_dzlog_conf -f $conf_dir/test_consumer_static_file_single.conf -n 10"
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
