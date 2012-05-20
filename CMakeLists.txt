cmake_minimum_required (VERSION 2.6)

project(fcitx-sayura)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# FIND_PACKAGE(Gettext REQUIRED)
find_package(Fcitx REQUIRED)
find_package(Gettext REQUIRED)

# uninstall target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")

option(ENABLE_SAYURA_DEBUG "Debug output for develop" OFF)

if(ENABLE_SAYURA_DEBUG)
  set(CMAKE_C_FLAGS "-DFCITX_SAYURA_DEBUG ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-DFCITX_SAYURA_DEBUG ${CMAKE_CXX_FLAGS}")
endif(ENABLE_SAYURA_DEBUG)

if(NOT DEFINED LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif(NOT DEFINED LIB_INSTALL_DIR)
if(NOT DEFINED LOCALEDIR)
  set(LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
endif(NOT DEFINED LOCALEDIR)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  IMMEDIATE @ONLY)

add_subdirectory(src)
add_subdirectory(data)
