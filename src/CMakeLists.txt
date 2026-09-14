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

add_executable(zlog-chk-conf zlog-chk-conf.c)
target_link_libraries(zlog-chk-conf zlog)

install(TARGETS
        zlog zlog_s zlog-chk-conf
        COMPONENT zlog
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
        zlog.h
        COMPONENT zlog
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# pkg-config support
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include")

if (WIN32)
    set(ZLOG_LIBS_PRIVATE "-lws2_32 -lunixem")
else ()
    set(ZLOG_LIBS_PRIVATE "-pthread")
endif ()

configure_file(zlog.pc.in ${CMAKE_CURRENT_BINARY_DIR}/zlog.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/zlog.pc
        COMPONENT zlog
        DESTINATION lib/pkgconfig
)
