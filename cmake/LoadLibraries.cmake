# =======================================================
# 支持多线程
# 对于需要多线程的库，使用以下命令包含连接库：
# target_link_libraries(xxx ${CMAKE_THREAD_PREFER_PTHREAD})
# =======================================================
if (Need_THREAD)
    # Threads::Threads carries -pthread as a compile option as well as a link
    # option, the way the makefile build passes it to both (src/Makefile), and
    # propagates it to everything linking zlog.
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    set(CMAKE_THREAD_PREFER_PTHREAD Threads::Threads)

    message(STATUS "thread lib : ${CMAKE_THREAD_PREFER_PTHREAD}")
endif (Need_THREAD)

if (Need_UNIXEM)
    find_package(Unixem)

    if (NOT UNIXEM_FOUND)
        message(FATAL_ERROR "unixem lib not found!")
    endif ()
endif ()
