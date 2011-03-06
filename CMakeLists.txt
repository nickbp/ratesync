cmake_minimum_required (VERSION 2.6)

project(ratesong)

set (ratesong_VERSION_MAJOR 1)
set (ratesong_VERSION_MINOR 0)
set (ratesong_VERSION_PATCH 0)

SET(SRCS
  config.in.h
  config.cpp
  linker.h
  linker.cpp
  main.cpp
  media-access.h
  media-access.cpp
  mpd-access.h
  mpd-access.cpp
  song.h
  symlink-access.h
  symlink-access.cpp
  tagger.h
  tagger.cpp)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.in.h"
  "${PROJECT_BINARY_DIR}/config.h"
  )
include_directories("${PROJECT_BINARY_DIR}")

add_executable(ratesong ${SRCS})

target_link_libraries(ratesong
  tag
  mpdclient)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${ratesong_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${ratesong_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${ratesong_VERSION_PATCH}")
include (CPack)
