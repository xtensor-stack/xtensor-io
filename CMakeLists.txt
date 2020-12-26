############################################################################
# Copyright (c) Wolf Vollprecht, Johan Mabille, and Sylvain Corlay         #
# Copyright (c) QuantStack                                                 #
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

set (xtensor_REQUIRED_VERSION 0.21.10)
find_package(xtensor ${xtensor_REQUIRED_VERSION} REQUIRED)

message(STATUS "Found xtensor: ${xtensor_INCLUDE_DIRS}/xtensor")

# Build
# =====

set(XTENSOR_IO_HEADERS
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xaudio.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xchunk_store_manager.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xfile_array.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xgdal.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xhighfive.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/ximage.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_binary.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_blosc.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_aws_handler.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_disk_handler.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_gcs_handler.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_gdal_handler.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_gzip.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_file.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_vsilfile.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xnpz.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor-io.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor_io_config.hpp
)

OPTION(BUILD_TESTS "xtensor-io test suite" OFF)
OPTION(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(DOWNLOAD_GBENCHMARK "download google benchmark and build from source" ON)

# Find OpenImageIO
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules")

add_library(xtensor-io INTERFACE)

include_directories(${GTEST_INCLUDE_DIRS})

target_include_directories(xtensor-io
    INTERFACE
    $<BUILD_INTERFACE:${XTENSOR_IO_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(xtensor-io
    INTERFACE
    xtensor
)

# We now check for each library seperately if they are installed. If not we
# print a warning and carry on!

message(STATUS "Trying to find OpenImageIO for ximage support")
find_package(OIIO)
if (${OIIO_FOUND})
  message(STATUS "OpenImageIO found, image file support enabled")
  include_directories(${OIIO_INCLUDE_DIRS})
  target_include_directories(xtensor-io
      INTERFACE
      $<BUILD_INTERFACE:${OIIO_INCLUDE_DIRS}>
  )
  target_link_libraries(xtensor-io
      INTERFACE
      ${OIIO_LIBRARIES}
  )
else()
    message(WARNING "OpenImageIO not found - install OpenImageIO for image support")
endif()

message(STATUS "Trying to find SndFile for xaudio support")
find_package(SndFile)
if (${LIBSNDFILE_FOUND})
  message(STATUS "SndFile found, audio file support enabled")
  include_directories(${LIBSNDFILE_INCLUDE_DIRS})
  target_include_directories(xtensor-io
      INTERFACE
      $<BUILD_INTERFACE:${LIBSNDFILE_INCLUDE_DIRS}>
  )
  target_link_libraries(xtensor-io
      INTERFACE
      ${LIBSNDFILE_LIBRARIES}
  )
else()
    message(WARNING "SndFile (libsndfile) not found - install libsndfile for audio support")
endif()

message(STATUS "Trying to find ZLIB for npz file support")
find_package(ZLIB)
if (${ZLIB_FOUND})
  message(STATUS "ZLIB found, npz file support enabled")
  include_directories(${ZLIB_INCLUDE_DIRS})
  target_include_directories(xtensor-io
      INTERFACE
      $<BUILD_INTERFACE:${ZLIB_INCLUDE_DIRS}>
  )
  target_link_libraries(xtensor-io
      INTERFACE
      ${ZLIB_LIBRARIES}
  )
else()
    message(WARNING "ZLIB not found - install zlib for xnpz file support")
endif()

message(STATUS "Trying to find HighFive for npz file support")
find_package(HighFive)
find_package(HDF5)
if (${HighFive_FOUND} AND ${HDF5_FOUND})
  message(STATUS "HighFive and HDF5 found, HDF5 file support enabled")
  find_package(HDF5 REQUIRED)
  get_target_property(HighFive_INCLUDE_DIRECTORIES_EXTRACTED HighFive INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(xtensor-io
      INTERFACE
      ${HighFive_INCLUDE_DIRECTORIES_EXTRACTED}
  )
  target_link_libraries(xtensor-io
      INTERFACE
      ${HDF5_LIBRARIES}
  )
else()
  message(WARNING "HighFive and/or HDF5 not found - install HighFive and HDF5 for HDF5 file support")
endif()

message(STATUS "Trying to find Blosc for Blosc file support")
find_package(Blosc)
if (${Blosc_FOUND})
  message(STATUS "Blosc found, Blosc file support enabled")
  include_directories(${Blosc_INCLUDE_DIRS})
  target_include_directories(xtensor-io
      INTERFACE
      $<BUILD_INTERFACE:${Blosc_INCLUDE_DIRS}>
  )
  target_link_libraries(xtensor-io
      INTERFACE
      ${Blosc_LIBRARIES}
  )
else()
    message(WARNING "Blosc not found - install blosc for Blosc file support")
endif()

message(STATUS "Trying to find GDAL for geospatial raster file support")
find_package(GDAL)
if(${GDAL_FOUND})
  message(STATUS "GDAL ${GDAL_VERSION} found, geospatial raster file support enabled")
  target_include_directories(xtensor-io
      INTERFACE
      ${GDAL_INCLUDE_DIRS}
  )
  target_link_libraries(xtensor-io
      INTERFACE
      ${GDAL_LIBRARIES}
  )
else()
  message(WARNING "GDAL not found - install GDAL for geospatial raster file support")
endif()

find_package(storage_client)
message(STATUS "Trying to find Google Cloud Storage for GCS IO handler support")
if(${storage_client_FOUND})
  message(STATUS "storage_client ${storage_client_VERSION} found, GCS IO handler support enabled")
  target_include_directories(xtensor-io
    INTERFACE
      ${storage_client_INCLUDE_DIRS}
  )
  target_link_libraries(xtensor-io
    INTERFACE
      storage_client
  )
else()
  message(WARNING "Google Cloud Storage not found - install google-cloud-cpp for GCS IO handler support")
endif()

if (NOT DEFINED BUILD_SHARED_LIBS)
  set (BUILD_SHARED_LIBS ON)
  find_package(AWSSDK COMPONENTS s3)
  unset (BUILD_SHARED_LIBS)
else ()
  find_package(AWSSDK COMPONENTS s3)
endif ()
message(STATUS "Trying to find AWS SDK for AWS S3 IO handler support")
if(${AWSSDK_FOUND})
  message(STATUS "AWSSDK ${AWSSDK_VERSION} found, AWS S3 IO handler support enabled")
  target_include_directories(xtensor-io
      INTERFACE
      ${AWSSDK_INCLUDE_DIRS}
  )
  target_link_libraries(xtensor-io
      INTERFACE
      ${AWSSDK_LINK_LIBRARIES}
  )
else()
  message(WARNING "AWSSDK not found - install aws-sdk-cpp for AWS S3 IO handler support")
endif()

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

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${XTENSOR_IO_CMAKECONFIG_INSTALL_DIR})

configure_file(${PROJECT_NAME}.pc.in
               "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
                @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig/")
