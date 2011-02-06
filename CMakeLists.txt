cmake_minimum_required (VERSION 2.6)

project(mpdtagger)

set (mpdtagger_VERSION_MAJOR 1)
set (mpdtagger_VERSION_MINOR 0)
set (mpdtagger_VERSION_PATCH 0)

SET(SRCS
  config.in.h
  config.cpp
  main.cpp
  mediaaccess.h
  mediaaccess.cpp
  mpdaccess.h
  mpdaccess.cpp
  tagger.h
  tagger.cpp)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.in.h"
  "${PROJECT_BINARY_DIR}/config.h"
  )
include_directories("${PROJECT_BINARY_DIR}")

add_executable(mpdtagger ${SRCS})

target_link_libraries(mpdtagger
  tag
  mpdclient)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE  
  "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${mpdtagger_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${mpdtagger_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${mpdtagger_VERSION_PATCH}")
include (CPack)
