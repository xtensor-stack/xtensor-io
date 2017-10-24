cmake_minimum_required(VERSION 2.6)
project(tutorial)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "../modules/")

find_library(xtensor REQUIRED)
find_library(OIIO REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -g")

include_directories(../include/ ../../xtensor/include)

add_executable(tutorial test.cpp)
# target_link_libraries(Tutorial ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
target_link_libraries(tutorial OpenImageIO z)