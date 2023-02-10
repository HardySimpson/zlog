#=======================================================
# 支持多线程
# 对于需要多线程的库，使用以下命令包含连接库：
# target_link_libraries(xxx ${CMAKE_THREAD_PREFER_PTHREAD})
#=======================================================
if(Need_THREAD)
    find_package(Threads REQUIRED)
    if(NOT CMAKE_THREAD_PREFER_PTHREAD) 
        set(CMAKE_THREAD_PREFER_PTHREAD ${CMAKE_THREAD_LIBS_INIT})
    endif()
    message(STATUS "thread lib : ${CMAKE_THREAD_PREFER_PTHREAD}")
endif(Need_THREAD)

if(Need_UNIXEM)
    find_package(Unixem)
    if (NOT UNIXEM_FOUND)
        message(FATAL_ERROR "unixem lib not found!")
    endif()
endif()
