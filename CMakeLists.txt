# LICENSE: Apache License 2.0
# Copyright (c) Hardy Simpson
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#/

cmake_minimum_required(VERSION 3.12)

message(STATUS "path : ${CMAKE_FIND_ROOT_PATH}")
project(zlog)
message(STATUS "path : ${CMAKE_FIND_ROOT_PATH}")

set(CMAKE_MODULE_PATH ${zlog_SOURCE_DIR}/cmake)

SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "18")
SET(CPACK_RPM_PACKAGE_RELEASE 1) # release version.
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(ZLOG_VERSION ${CPACK_PACKAGE_VERSION})
SET(ZLOG_SO_VERSION ${CPACK_PACKAGE_VERSION_MAJOR})

message(STATUS "platform : ${CMAKE_SYSTEM}")

add_definitions("-g -Wall -Wstrict-prototypes")
set(CMAKE_C_FLAGS "-std=c11 -pedantic -D_DEFAULT_SOURCE")
set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif()

if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 ")
endif ()

cmake_policy(SET CMP0015 NEW)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${zlog_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${zlog_BINARY_DIR}/lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${zlog_BINARY_DIR}/bin")

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(Need_THREAD 1)

if (WIN32)
    set(Need_UNIXEM 1)
endif ()

include(cmake/LoadLibraries.cmake)

add_subdirectory(src)
add_subdirectory(cpack)

if (UNIT_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()
