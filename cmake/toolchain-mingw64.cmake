# this one is important
SET(CMAKE_SYSTEM_NAME Windows)

# specify the cross compiler
find_path(MINGW_BIN_PATH gcc.exe PATHS c:/mingw64 d:/mingw64 c:/mingw-build/mingw64 d:/mingw-build/mingw64 PATH_SUFFIXES bin)

if (MINGW_PATH_NOTFOUND)
    message(FATAL "mingw64 not found!")
endif ()

get_filename_component(MINGW_PATH ${MINGW_BIN_PATH} PATH)

SET(CMAKE_GENERATOR "MinGW Makefiles")

SET(CMAKE_C_COMPILER gcc)
SET(CMAKE_CXX_COMPILER g++)

SET(CMAKE_RC_COMPILER windres)
set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")

# SET(_CMAKE_TOOLCHAIN_PREFIX x86_64-w64-mingw32-)
SET(_CMAKE_TOOLCHAIN_LOCATION ${MINGW_BIN_PATH})

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH ${MINGW_PATH} ${MINGW_PATH}/x86_64-w64-mingw32)

SET(CMAKE_SYSTEM_INCLUDE_PATH "${CMAKE_SYSTEM_INCLUDE_PATH} ${MINGW_PATH}/include ${MINGW_PATH}/x86_64-w64-mingw32/include")
SET(CMAKE_SYSTEM_LIBRARY_PATH "${CMAKE_SYSTEM_LIBRARY_PATH} ${MINGW_PATH}/lib ${MINGW_PATH}/x86_64-w64-mingw32/lib")
SET(CMAKE_SYSTEM_PROGRAM_PATH "${CMAKE_SYSTEM_PROGRAM_PATH} ${MINGW_PATH}/bin ${MINGW_PATH}/x86_64-w64-mingw32/bin")

# printf support %sz format.
add_definitions("-D_POSIX")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
