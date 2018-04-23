############################################################################
# Copyright (c) 2016, Wolf Vollprecht, Johan Mabille, and Sylvain Corlay   #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.8)
project(xtensor-io)

set(XTENSOR_IO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Configuration
# =============

# TODO: use the library directory of the found LIBS implementation instead of CMAKE_INSTALL_PREFIX
set(XTENSOR_IO_CLING_LIBRARY_DIR_64 "\"${CMAKE_INSTALL_PREFIX}/lib64\"")
set(XTENSOR_IO_CLING_LIBRARY_DIR_32 "\"${CMAKE_INSTALL_PREFIX}/lib32\"")
set(XTENSOR_IO_CLING_LIBRARY_DIR "\"${CMAKE_INSTALL_PREFIX}/lib\"")

configure_file (
    "${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor_io_config.hpp.in"
    "${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor_io_config.hpp"
)

# Versionning
# ===========

file(STRINGS "${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor_io_config.hpp" xtensor_io_version_defines
     REGEX "#define XTENSOR_IO_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xtensor_io_version_defines})
    if(ver MATCHES "#define XTENSOR_IO_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XTENSOR_IO_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${XTENSOR_IO_VERSION_MAJOR}.${XTENSOR_IO_VERSION_MINOR}.${XTENSOR_IO_VERSION_PATCH})
message(STATUS "Building xtensor-io v${${PROJECT_NAME}_VERSION}")

# Dependencies
# ============

find_package(xtensor REQUIRED)
find_package(xtl REQUIRED)

message(STATUS "Found xtensor: ${xtensor_INCLUDE_DIRS}/xtensor")

# Build
# =====

set(XTENSOR_IO_HEADERS
  ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/ximage.hpp
  ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xaudio.hpp
  ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xnpz.hpp
  ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor_io_config.hpp
)

OPTION(BUILD_TESTS "xtensor-io test suite" OFF)
OPTION(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(DOWNLOAD_GBENCHMARK "download google benchmark and build from source" ON)

# Find OpenImageIO
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules")

find_package(OIIO REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SndFile REQUIRED)

include_directories(${OIIO_INCLUDE_DIRS})
include_directories(${LIBSNDFILE_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

add_library(xtensor-io INTERFACE)

target_include_directories(xtensor-io
  INTERFACE
    $<BUILD_INTERFACE:${XTENSOR_IO_INCLUDE_DIR}>
    # LIBRARIES
    $<BUILD_INTERFACE:${OIIO_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${LIBSNDFILE_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${ZLIB_INCLUDE_DIRS}>
    # INSTALLATION
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(xtensor-io
  INTERFACE
    xtl
    xtensor
    ${OIIO_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${LIBSNDFILE_LIBRARIES}
)

if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(BUILD_TESTS ON)
endif()

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS xtensor-io
        EXPORT ${PROJECT_NAME}-targets)

export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

install(FILES ${XTENSOR_IO_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xtensor-io)

install(DIRECTORY ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/thirdparty/zstr
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xtensor-io/thirdparty)

set(XTENSOR_IO_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE
    STRING "install path for xtensor-ioConfig.cmake")

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XTENSOR_IO_CMAKECONFIG_INSTALL_DIR})

# xtensor is header-only and does not depend on the architecture.
# Remove CMAKE_SIZEOF_VOID_P from xtensorConfigVersion.cmake so that an xtensorConfig.cmake
# generated for a 64 bit target can be used for 32 bit targets and vice versa.
set(_XTENSOR_IO_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
unset(CMAKE_SIZEOF_VOID_P)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
set(CMAKE_SIZEOF_VOID_P ${_XTENSOR_IO_CMAKE_SIZEOF_VOID_P})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${XTENSOR_IO_CMAKECONFIG_INSTALL_DIR})

configure_file(${PROJECT_NAME}.pc.in
               "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
                @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")
