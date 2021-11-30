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

set (xtensor_REQUIRED_VERSION 0.23.0)
if(TARGET xtensor)
    set(xtensor_VERSION ${XTENSOR_VERSION_MAJOR}.${XTENSOR_VERSION_MINOR}.${XTENSOR_VERSION_PATCH})
    if( NOT ${xtensor_VERSION} VERSION_GREATER_EQUAL ${xtensor_REQUIRED_VERSION})
        message(ERROR "Mismatch xtensor versions. Found '${xtensor_VERSION}' but requires: '${xtensor_REQUIRED_VERSION}'")
    else()
        message(STATUS "Found xtensor v${xtensor_VERSION}")
    endif()
else()
    find_package(xtensor ${xtensor_REQUIRED_VERSION} REQUIRED)
    message(STATUS "Found xtensor: ${xtensor_INCLUDE_DIRS}/xtensor")
endif()


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
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_zlib.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_file_wrapper.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_vsilfile_wrapper.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xio_stream_wrapper.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xnpz.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor-io.hpp
    ${XTENSOR_IO_INCLUDE_DIR}/xtensor-io/xtensor_io_config.hpp
)

OPTION(BUILD_TESTS "xtensor-io test suite" OFF)
OPTION(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(DOWNLOAD_GBENCHMARK "download google benchmark and build from source" ON)

# each optional dependency can be required individually with e.g. -DHAVE_OIIO=ON

OPTION(HAVE_OIIO "require OpenImageIO dependency for image file support" OFF)
OPTION(HAVE_SndFile "require SndFile for audio file support" OFF)
OPTION(HAVE_ZLIB "require ZLIB for npz file support" OFF)
OPTION(HAVE_HighFive "require HighFive for HDF5 file support" OFF)
OPTION(HAVE_Blosc "require Blosc for Blosc file support" OFF)
OPTION(HAVE_GDAL "require GDAL for geospatial raster file support" OFF)
OPTION(HAVE_storage_client "require storage_client for Google Cloud Storage IO handler support" OFF)
OPTION(HAVE_AWSSDK "require AWSSDK for AWS S3 IO handler support" OFF)

# all dependencies can be required with -DHAVE_ALL_DEPS=ON

OPTION(HAVE_ALL_DEPS "require all optional dependencies" OFF)

if(HAVE_ALL_DEPS)
  set(HAVE_OIIO ON)
  set(HAVE_SndFile ON)
  set(HAVE_ZLIB ON)
  set(HAVE_HighFive ON)
  set(HAVE_Blosc ON)
  set(HAVE_GDAL ON)
  set(HAVE_storage_client ON)
  set(HAVE_AWSSDK ON)
endif()

# a list of dependencies can be required with e.g. "-DOPTIONAL_DEPENDENCIES=OIIO;SndFile"

foreach(DEP ${OPTIONAL_DEPENDENCIES})
  set(HAVE_${DEP} ON)
endforeach()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules")

add_library(xtensor-io INTERFACE)

include_directories(${GTEST_INCLUDE_DIRS})

find_path(ghc-filesystem-include REQUIRED
    NAMES "ghc/filesystem.hpp"
    DOC "Path to include directory of https://github.com/gulrak/filesystem"
    )
target_include_directories(xtensor-io
    INTERFACE
    $<BUILD_INTERFACE:${ghc-filesystem-include}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(xtensor-io
    INTERFACE
    $<BUILD_INTERFACE:${XTENSOR_IO_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(xtensor-io
    INTERFACE
    xtensor
)

# We now check for each optional library seperately if they are required.

if(HAVE_OIIO)
  find_package(OIIO REQUIRED)
  message(STATUS "OpenImageIO ${OIIO_VERSION} found, image file support enabled")
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
  message(STATUS "OpenImageIO not enabled: use -DHAVE_OIIO=ON for image file support")
endif()

if(HAVE_SndFile)
  find_package(SndFile REQUIRED)
  message(STATUS "SndFile ${SndFile_VERSION} found, audio file support enabled")
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
  message(STATUS "SndFile not enabled: use -DHAVE_SndFile=ON for audio file support")
endif()

if(HAVE_ZLIB)
  find_package(ZLIB REQUIRED)
  message(STATUS "ZLIB ${ZLIB_VERSION} found, npz file support enabled")
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
  message(STATUS "ZLIB not enabled: use -DHAVE_ZLIB=ON for npz file support")
endif()

if(HAVE_HighFive)
  find_package(HighFive REQUIRED)
  find_package(HDF5 REQUIRED)
  message(STATUS "HighFive ${HighFive_VERSION} and HDF5 ${HDF5_VERSION} found, HDF5 file support enabled")
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
  message(STATUS "HighFive not enabled: use -DHAVE_HighFive=ON for HDF5 file support")
endif()

if(HAVE_Blosc)
  find_package(Blosc REQUIRED)
  message(STATUS "Blosc ${Blosc_VERSION} found, Blosc file support enabled")
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
  message(STATUS "Blosc not enabled: use -DHAVE_Blosc=ON for Blosc file support")
endif()

if(HAVE_GDAL)
  find_package(GDAL REQUIRED)
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
  message(STATUS "GDAL not enabled: use -DHAVE_GDAL=ON for geospatial raster file support")
endif()

if(HAVE_storage_client)
  find_package(storage_client REQUIRED)
  message(STATUS "storage_client ${storage_client_VERSION} found, Google Cloud Storage IO handler support enabled")
  target_include_directories(xtensor-io
    INTERFACE
      ${storage_client_INCLUDE_DIRS}
  )
  target_link_libraries(xtensor-io
    INTERFACE
      storage_client
  )
else()
  message(STATUS "storage_client not enabled: use -DHAVE_storage_client=ON for Google Cloud Storage IO handler support")
endif()

if(HAVE_AWSSDK)
  if (NOT DEFINED BUILD_SHARED_LIBS)
    set (BUILD_SHARED_LIBS ON)
    find_package(AWSSDK COMPONENTS s3 REQUIRED)
    unset (BUILD_SHARED_LIBS)
  else ()
    find_package(AWSSDK COMPONENTS s3 REQUIRED)
  endif ()
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
  message(STATUS "AWSSDK not enabled: use -DHAVE_AWSSDK=ON for AWS S3 IO handler support")
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
