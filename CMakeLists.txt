cmake_minimum_required (VERSION 2.6)

project(ratesync)

set (ratesync_VERSION_MAJOR 1)
set (ratesync_VERSION_MINOR 0)
set (ratesync_VERSION_PATCH 0)

SET(SRCS
  config.in.h
  config.cpp
  main.cpp
  sink.h
  sink-file.h
  sink-file.cpp
  sink-mpd.h
  sink-mpd.cpp
  sink-symlink.h
  sink-symlink.cpp
  song.h
  updater.h
  updater.cpp)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.in.h"
  "${PROJECT_BINARY_DIR}/config.h"
  )
include_directories("${PROJECT_BINARY_DIR}")

add_executable(ratesync ${SRCS})

target_link_libraries(ratesync
  tag
  mpdclient)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${ratesync_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${ratesync_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${ratesync_VERSION_PATCH}")
include (CPack)
