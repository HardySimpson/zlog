# create by lsm <lsm@skybility.com>

#===============================
# package info setting
#===============================
SET(CPACK_PACKAGE_NAME "zlog")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "log component for Linux/Unix/AIX")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_HOME_DIRECTORY}/README")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${zlog_BINARY_DIR};zlog;zlog;/")

#=================================
# dependency setting
#=================================
#SET(CPACK_RPM_PACKAGE_REQUIRES "")
#SET(CPACK_DEBIAN_PACKAGE_DEPENDS "")

#===============================
# copy file to build directory.
#===============================
SET(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/CPackConfig.cmake")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../CPackConfig.cmake CPackConfig.cmake)

file(COPY 
    .

    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}

    PATTERN CMakeLists.txt EXCLUDE
    )

