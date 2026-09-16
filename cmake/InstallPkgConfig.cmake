# Runs at install time, from install(SCRIPT), so that CMAKE_INSTALL_PREFIX is
# the prefix actually being installed to -- including one given late with
# `cmake --install <dir> --prefix <p>`, which a configure time configure_file()
# cannot see. The caller passes the ZLOG_* variables via install(CODE).

set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/${ZLOG_INSTALL_LIBDIR}")
set(includedir "\${prefix}/${ZLOG_INSTALL_INCLUDEDIR}")
set(ZLOG_VERSION "${ZLOG_VERSION}")
set(ZLOG_LIBS_PRIVATE "${ZLOG_LIBS_PRIVATE}")

configure_file("${ZLOG_PC_IN}" "${ZLOG_PC_OUT}" @ONLY)
