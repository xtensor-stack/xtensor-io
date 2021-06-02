############################################################################
# Copyright (c) Wolf Vollprecht, Johan Mabille, and Sylvain Corlay         #
# Copyright (c) QuantStack                                                 #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.8)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(xtensor-io-test)

    find_package(xtensor-io REQUIRED CONFIG)
    set(XTENSOR_IO_INCLUDE_DIR ${xtensor_INCLUDE_DIRS})
endif ()

message(STATUS "Forcing tests build type to Release")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wunused-parameter -Wextra -Wreorder -Wconversion -Wsign-conversion")
    CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)

    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "Unsupported compiler -- xtensor requires C++14 support!")
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP /bigobj")
    set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
endif()

if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    if(DOWNLOAD_GTEST)
        # Download and unpack googletest at configure time
        configure_file(downloadGTest.cmake.in googletest-download/CMakeLists.txt)
    else()
        # Copy local source of googletest at configure time
        configure_file(copyGTest.cmake.in googletest-download/CMakeLists.txt)
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                     ${CMAKE_CURRENT_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)

    set(GTEST_INCLUDE_DIRS "${gtest_SOURCE_DIR}/include")
    set(GTEST_BOTH_LIBRARIES  gtest_main gtest)
else()
    find_package(GTest REQUIRED)
endif()

find_package(Threads)

set(XTENSOR_IO_TESTS main.cpp test_xio_binary.cpp)
set(XTENSOR_IO_FILES)

if (HAVE_GDAL)
    list(APPEND XTENSOR_IO_TESTS test_xgdal.cpp test_xio_gdal_handler.cpp)
endif()

if (HAVE_ZLIB)
    list(APPEND XTENSOR_IO_TESTS test_xnpz.cpp test_xio_zlib.cpp test_xio_gzip.cpp)
    list(
        APPEND
        XTENSOR_IO_FILES
        compressed.npz
        uncompressed.npz
        compressed64.npz
        uncompressed64.npz
        test.gz
    )
endif()

if (HAVE_HighFive)
    list(APPEND XTENSOR_IO_TESTS test_xhighfive.cpp)
    list(APPEND XTENSOR_IO_FILES archive.h5)
endif()

if (HAVE_OIIO)
    list(APPEND XTENSOR_IO_TESTS test_ximage.cpp)
    list(
        APPEND
        XTENSOR_IO_FILES
        test.png
        test.jpg
        test.gif
        big.jpg
    )
endif()

if (HAVE_SndFile)
    list(APPEND XTENSOR_IO_TESTS test_xaudio.cpp)
    list(APPEND XTENSOR_IO_FILES xtensor.wav)
endif()

if (HAVE_Blosc)
    list(APPEND XTENSOR_IO_TESTS test_xio_blosc.cpp)
    list(APPEND XTENSOR_IO_FILES test.blosc)
endif()

if (HAVE_storage_client)
    list(APPEND XTENSOR_IO_TESTS test_xio_gcs_handler.cpp)
endif()

if (HAVE_AWSSDK)
    list(APPEND XTENSOR_IO_TESTS test_xio_aws_handler.cpp)
endif()

set(XTENSOR_IO_HO_TESTS
    main.cpp
    test_xchunk_store_manager.cpp
    test_xfile_array.cpp
)

# Add files for tests
foreach(filename IN LISTS XTENSOR_IO_FILES)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/files/${filename}
        ${CMAKE_CURRENT_BINARY_DIR}/files/${filename} COPYONLY)
endforeach()

add_executable(test_xtensor_io_lib ${XTENSOR_IO_TESTS} ${XTENSOR_IO_HEADERS})
if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    add_dependencies(test_xtensor_io_lib gtest_main)
endif()

if(DEFINED CMAKE_INSTALL_PREFIX)
  set_target_properties(test_xtensor_io_lib
    PROPERTIES
      INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
      BUILD_WITH_INSTALL_RPATH ON
  )
endif()

target_link_libraries(test_xtensor_io_lib
  PUBLIC
    xtensor-io
    ${CMAKE_DL_LIBS}
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(test_xtensor_io_lib PRIVATE ${XTENSOR_IO_INCLUDE_DIR})

add_executable(test_xtensor_io_ho ${XTENSOR_IO_HO_TESTS} ${XTENSOR_IO_HEADERS})
target_include_directories(test_xtensor_io_ho PRIVATE ${XTENSOR_IO_INCLUDE_DIR})
target_link_libraries(test_xtensor_io_ho
  PUBLIC
    xtensor-io
    ${CMAKE_DL_LIBS}
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

add_custom_target(xtest
    COMMAND test_xtensor_io_lib
    COMMAND test_xtensor_io_ho
    DEPENDS test_xtensor_io_lib test_xtensor_io_ho
)

foreach(filename IN LISTS XTENSOR_IO_HO_TESTS)
    string(REPLACE ".cpp" "" targetname ${filename})
    add_executable(${targetname} ${filename} ${XTENSOR_IO_HEADERS})
    target_include_directories(${targetname} PRIVATE ${XTENSOR_IO_INCLUDE_DIR})
    target_link_libraries(${targetname}
        PUBLIC
            xtensor-io
            ${CMAKE_DL_LIBS}
            ${GTEST_BOTH_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
    )
    add_custom_target(
        x${targetname}
        COMMAND ${targetname}
        DEPENDS ${targetname} ${filename} ${XTENSOR_IO_HEADERS}
    )
endforeach()
