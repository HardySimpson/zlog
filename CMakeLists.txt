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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for compile commands used by lsp

message(STATUS "path : ${CMAKE_FIND_ROOT_PATH}")
project(zlog LANGUAGES C)
message(STATUS "path : ${CMAKE_FIND_ROOT_PATH}")

set(CMAKE_MODULE_PATH ${zlog_SOURCE_DIR}/cmake)

# The version is defined once, in src/version.h, and parsed by both build
# systems (see src/Makefile) so the library, the file names and the packages
# can never disagree.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS src/version.h)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h" ZLOG_VERSION_LINE
        REGEX "^#define[ \t]+ZLOG_VERSION[ \t]+\"[0-9]+\\.[0-9]+\\.[0-9]+\"")
string(REGEX REPLACE "^.*\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*$" "\\1" ZLOG_VERSION "${ZLOG_VERSION_LINE}")

if (NOT ZLOG_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "cannot parse ZLOG_VERSION from src/version.h")
endif ()

string(REPLACE "." ";" ZLOG_VERSION_PARTS ${ZLOG_VERSION})
list(GET ZLOG_VERSION_PARTS 0 CPACK_PACKAGE_VERSION_MAJOR)
list(GET ZLOG_VERSION_PARTS 1 CPACK_PACKAGE_VERSION_MINOR)
list(GET ZLOG_VERSION_PARTS 2 CPACK_PACKAGE_VERSION_PATCH)
SET(CPACK_RPM_PACKAGE_RELEASE 1) # release version.
SET(CPACK_PACKAGE_VERSION ${ZLOG_VERSION})

# soname is libzlog.so.MAJOR.MINOR, as shipped by the makefile build since 1.2
SET(ZLOG_SO_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")

message(STATUS "version : ${ZLOG_VERSION} (soname ${ZLOG_SO_VERSION})")

message(STATUS "platform : ${CMAKE_SYSTEM}")

add_definitions("-g -Wall -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -D_DEFAULT_SOURCE")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 ")
endif ()

# lib vs lib64, bin, include: let the platform decide, the way the makefile
# build lets LIBRARY_PATH/BINARY_PATH/INCLUDE_PATH decide
include(GNUInstallDirs)

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
