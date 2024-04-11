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

set_target_properties(zlog_s PROPERTIES OUTPUT_NAME zlog)

add_executable(zlog-chk-conf zlog-chk-conf.c)
target_link_libraries(zlog-chk-conf zlog)

install(TARGETS
        zlog zlog_s zlog-chk-conf
        COMPONENT zlog
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

install(FILES
        zlog.h
        COMPONENT zlog
        DESTINATION include
)
