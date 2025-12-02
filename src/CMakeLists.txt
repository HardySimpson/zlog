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

if (CMAKE_BUILD_TYPE STREQUAL "Asan")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(zlog PRIVATE -fsanitize=address -g -fno-omit-frame-pointer)
        target_link_options(zlog PRIVATE -fsanitize=address)
    endif()
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

if (CMAKE_BUILD_TYPE STREQUAL "Tsan")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(zlog PRIVATE -fsanitize=thread -g -O1  -fno-omit-frame-pointer)
        target_link_options(zlog PRIVATE -fsanitize=thread)
        target_compile_options(zlog-chk-conf PRIVATE -fsanitize=thread -g -O1  -fno-omit-frame-pointer)
        target_link_options(zlog-chk-conf PRIVATE -fsanitize=thread)
    endif()
endif()
