aux_source_directory(. SRCS)

if (NOT WIN32)
    list(REMOVE_ITEM SRCS ./zlog_win.c)
endif ()

list(REMOVE_ITEM SRCS ./zlog-chk-conf.c)

add_library(zlog
        SHARED
        ${SRCS}
)
target_link_libraries(zlog
        ${CMAKE_THREAD_PREFER_PTHREAD}
)

if (WIN32)
    target_link_libraries(zlog
            ${UNIXEM_LIBRARY}
            Ws2_32
    )
endif ()

set_target_properties(zlog PROPERTIES VERSION ${ZLOG_VERSION} SOVERSION ${ZLOG_SO_VERSION})
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(zlog PROPERTIES C_VISIBILITY_PRESET hidden)
endif()

add_library(zlog_s
        STATIC
        ${SRCS}
)
target_link_libraries(zlog_s
        ${CMAKE_THREAD_PREFER_PTHREAD}
)

if (WIN32)
    target_link_libraries(zlog_s
            ${UNIXEM_LIBRARY}
            Ws2_32
    )
endif ()

# -fPIC, like the makefile build: the static library must stay linkable into
# a shared object
set_target_properties(zlog_s PROPERTIES
        OUTPUT_NAME zlog
        POSITION_INDEPENDENT_CODE ON
)

foreach (target zlog zlog_s)
    target_include_directories(${target} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endforeach ()

add_executable(zlog-chk-conf zlog-chk-conf.c)
target_link_libraries(zlog-chk-conf zlog)

install(TARGETS
        zlog zlog_s
        EXPORT zlogTargets
        COMPONENT zlog
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS
        zlog-chk-conf
        COMPONENT zlog
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
        zlog.h
        COMPONENT zlog
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# cmake package support: find_package(zlog) and the zlog::zlog target
set(ZLOG_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/zlog)

install(EXPORT zlogTargets
        FILE zlogTargets.cmake
        NAMESPACE zlog::
        COMPONENT zlog
        DESTINATION ${ZLOG_CMAKE_DIR}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(zlogConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/zlogConfig.cmake
        INSTALL_DESTINATION ${ZLOG_CMAKE_DIR}
)

# the soname is MAJOR.MINOR, so a minor bump is an abi break here
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/zlogConfigVersion.cmake
        VERSION ${ZLOG_VERSION}
        COMPATIBILITY SameMinorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/zlogConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/zlogConfigVersion.cmake
        COMPONENT zlog
        DESTINATION ${ZLOG_CMAKE_DIR}
)

# pkg-config support
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")

if (WIN32)
    set(ZLOG_LIBS_PRIVATE "-lws2_32 -lunixem")
else ()
    set(ZLOG_LIBS_PRIVATE "-pthread")
endif ()

configure_file(zlog.pc.in ${CMAKE_CURRENT_BINARY_DIR}/zlog.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/zlog.pc
        COMPONENT zlog
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
