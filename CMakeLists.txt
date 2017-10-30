############################################################################
# Copyright (c) 2016, Wolf Vollprecht, Johan Mabille, and Sylvain Corlay   #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.1)
project(xtensor-io)

set(XTENSOR_IO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
message(STATUS "Found xtensor: ${xtensor_INCLUDE_DIRS}/xtensor")

# Find OpenImageIO
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules")
message("MODULE PATH: ${CMAKE_MODULE_PATH}")

find_package(OIIO)
message(STATUS "Found OpenImageIO: ${OIIO_INCLUDE_DIRS}")

find_package(ZLIB)
message(STATUS "Found zlib: ${zlib_INCLUDE_DIRS}")

# Build
# =====

set(XTENSOR_IO_HEADERS
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/ximageio.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xnpz.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor_io_config.hpp
)

OPTION(BUILD_TESTS "xtensor-io test suite" OFF)
# OPTION(BUILD_BENCHMARK "xtensor-io benchmark" OFF)
OPTION(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(DOWNLOAD_GBENCHMARK "download google benchmark and build from source" ON)

if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(BUILD_TESTS ON)
endif()

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

if(BUILD_BENCHMARK)
    add_subdirectory(benchmark)
endif()

include_directories(${xtensor_INCLUDE_DIRS})

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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
