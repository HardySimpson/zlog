# create by lsm <lsm@skybility.com>
# cmake

cmake_minimum_required(VERSION 2.8.5)

message(STATUS "path : ${CMAKE_FIND_ROOT_PATH}")
project(zlog)
message(STATUS "path : ${CMAKE_FIND_ROOT_PATH}")

set(CMAKE_MODULE_PATH ${zlog_SOURCE_DIR}/cmake)

#=====================================
# version of zlog
#=====================================
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "12")
SET(CPACK_RPM_PACKAGE_RELEASE 1)    #release version.
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(zlog_ver ${CPACK_PACKAGE_VERSION})
SET(zlog_so_ver ${CPACK_PACKAGE_VERSION_MAJOR})

#=======================================================

message(STATUS "plateform : ${CMAKE_SYSTEM}")

add_definitions("-g -Wall -Wstrict-prototypes")
set(CMAKE_C_FLAGS "-std=c99 -pedantic -D_DEFAULT_SOURCE")
set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")

if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 ")
endif()

#=====================================================
# include dir
# include_directories(include)
#=====================================================

#=====================================================
# lib output path.
cmake_policy(SET CMP0015 NEW)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${zlog_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${zlog_BINARY_DIR}/lib")
# bin output path.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${zlog_BINARY_DIR}/bin")
#=====================================================

#=====================================================
# link path.
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
#=====================================================

#=====================================================
# library depend.
set(Need_THREAD 1)
if (WIN32)
    set(Need_UNIXEM 1)
endif()

include(cmake/LoadLibraries.cmake)
#=====================================================

#========================================================
# sub dir
add_subdirectory(src)
add_subdirectory(cpack)
#========================================================

#========================================================
# for unittest, call "cmake .. -DUNIT_TEST=on"
if(UNIT_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
#========================================================

