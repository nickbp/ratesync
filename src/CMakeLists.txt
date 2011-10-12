cmake_minimum_required (VERSION 2.6)

project(ratesync)

find_path(taglib_INCLUDE_DIR NAMES taglib/taglib.h)
find_library(taglib_LIBRARY NAMES tag)
find_path(mpdclient_INCLUDE_DIR NAMES mpd/client.h)
find_library(mpdclient_LIBRARY NAMES mpdclient)

if(taglib_INCLUDE_DIR AND taglib_LIBRARY)
  message(STATUS "Found taglib")
  set(FOUND_TAGLIB ON)
else()
  message(STATUS "Didn't find taglib")
endif()
if(mpdclient_INCLUDE_DIR AND mpdclient_LIBRARY)
  message(STATUS "Found libmpdclient")
  set(FOUND_MPDCLIENT ON)
else()
  message(STATUS "Didn't find libmpdclient")
endif()

option(USE_MPDCLIENT "Use libmpdclient" ${FOUND_MPDCLIENT})

set (ratesync_VERSION_MAJOR 1)
set (ratesync_VERSION_MINOR 0)
set (ratesync_VERSION_PATCH 0)

SET(SRCS
  config.in.h
  config.cpp
  main.cpp
  sink.h
  sink-symlink.h
  sink-symlink.cpp
  #lib-dependent sinks added below
  song.h
  updater.h
  updater.cpp)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.in.h"
  "${PROJECT_BINARY_DIR}/config.h"
  )

if(FOUND_TAGLIB)
  list(APPEND INCLUDES ${taglib_INCLUDE_DIR})
  list(APPEND LIBS ${taglib_LIBRARY})
  list(APPEND SRCS sink-file.h sink-file.cpp)
else()
  message(ERROR "Required taglib not found. Install libtag-dev and re-configure.")
endif()

if(USE_MPDCLIENT)
  message(STATUS "Using libmpdclient")
  list(APPEND INCLUDES ${mpdclient_INCLUDE_DIR})
  list(APPEND LIBS ${mpdclient_LIBRARY})
  list(APPEND SRCS sink-mpd.h sink-mpd.cpp)
else()
  message(STATUS "Not using libmpdclient")
endif()

include_directories(${PROJECT_BINARY_DIR} ${INCLUDES})
add_executable(ratesync ${SRCS})
target_link_libraries(ratesync ${LIBS})

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/../LICENCE")
set (CPACK_PACKAGE_VERSION_MAJOR "${ratesync_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${ratesync_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${ratesync_VERSION_PATCH}")
include (CPack)
