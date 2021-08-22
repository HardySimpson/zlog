cmake_minimum_required (VERSION 3.4.1)

project (zlog)

include_directories(${PROJECT_SOURCE_DIR}/src)

aux_source_directory(src DIR_SRCS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -O2 -pthread")

add_library(zlog SHARED ${DIR_SRCS})


