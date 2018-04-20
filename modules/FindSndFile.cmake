# Copyright: Apache License, Audaspace Project
# Source: https://github.com/neXyon/audaspace/blob/d27746ca8550e2adae6ba54ded0c6dfac58d69b8/cmake/FindLibSndFile.cmake

# Try to find libsndfile
# Once done, this will define
#
#  LIBSNDFILE_FOUND - system has libsndfile
#  LIBSNDFILE_INCLUDE_DIRS - the libsndfile include directories
#  LIBSNDFILE_LIBRARIES - link these to use libsndfile

# Include dir

set(_LIBSNDFILE_HINTS
    "${LIBSNDFILE_HINTS}"
    "${CMAKE_INSTALL_PREFIX}"
)

find_path(LIBSNDFILE_INCLUDE_DIR
    NAMES sndfile.h
    HINTS ${_LIBSNDFILE_HINTS}
)

# Library
find_library(LIBSNDFILE_LIBRARY
    NAMES sndfile
    HINTS ${_LIBSNDFILE_HINTS}
)

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(LibSndFile  DEFAULT_MSG  LIBSNDFILE_LIBRARY LIBSNDFILE_INCLUDE_DIR)

if(LIBSNDFILE_FOUND)
  set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBRARY})
  set(LIBSNDFILE_INCLUDE_DIRS ${LIBSNDFILE_INCLUDE_DIR})
endif(LIBSNDFILE_FOUND)

mark_as_advanced(LIBSNDFILE_LIBRARY LIBSNDFILE_LIBRARIES LIBSNDFILE_INCLUDE_DIR LIBSNDFILE_INCLUDE_DIRS)

unset(_LIBSNDFILE_HINTS)