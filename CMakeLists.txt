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

# warnings for every config; debug info is left to the per config flags below,
# so Release is optimised without it and RelWithDebInfo keeps it, rather than
# every config carrying -g because add_definitions() cannot tell them apart
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -D_DEFAULT_SOURCE")

# defined signed overflow, as in the makefile build (src/Makefile)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fwrapv")

# -pthread at compile time, not just at link time: the makefile build passes it
# to both, and on a glibc new enough to keep pthread_create in libc FindThreads
# resolves the link side with no flag at all, which would leave the two builds
# compiling the sources in different states (_REENTRANT and friends).
include(CheckCCompilerFlag)
check_c_compiler_flag(-pthread ZLOG_HAVE_PTHREAD_FLAG)
if (ZLOG_HAVE_PTHREAD_FLAG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
endif ()

# the makefile build is -Werror everywhere except mingw; keep that, but let
# packagers building with a compiler we have not seen turn it off
option(ZLOG_WERROR "treat compiler warnings as errors" ON)
if (ZLOG_WERROR AND NOT MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif ()
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

# Sanitizer build types, used by the CI matrix and documented in
# doc/developer.md: cmake -DCMAKE_BUILD_TYPE=Tsan|Asan
set(CMAKE_C_FLAGS_TSAN "-g -O1 -fno-omit-frame-pointer -fsanitize=thread")
set(CMAKE_EXE_LINKER_FLAGS_TSAN "-fsanitize=thread")
set(CMAKE_SHARED_LINKER_FLAGS_TSAN "-fsanitize=thread")
set(CMAKE_MODULE_LINKER_FLAGS_TSAN "-fsanitize=thread")

set(CMAKE_C_FLAGS_ASAN "-g -O1 -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS_ASAN "-fsanitize=address")
set(CMAKE_SHARED_LINKER_FLAGS_ASAN "-fsanitize=address")
set(CMAKE_MODULE_LINKER_FLAGS_ASAN "-fsanitize=address")

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel Tsan Asan)

# these two only exist for gcc and clang: say so instead of silently building
# an unsanitized library, which is what an undefined build type would do
if (CMAKE_BUILD_TYPE MATCHES "^(Tsan|Asan)$"
        AND NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR
            "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} needs gcc or clang, "
            "not ${CMAKE_C_COMPILER_ID}")
endif ()

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
