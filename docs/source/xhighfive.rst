.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

HDF5 interface
==============

Basic interface
---------------

The basic interface consists of two functions:

*   ``xt::dump_hdf5(filename, path, data[, xt::file_mode, xt::dump_mode])``

    Dump data (e.g. a matrix) to a DataSet in a HDF5 file. The data can be: ``scalar`` (incl. ``std::string``), ``std::vector<scalar>`` (incl. ``std::string``), ``xt::xarray<scalar>``, and ``xt::xtensor<scalar,dim>``.

*   ``data = xt::load_hdf5<...>(filename, path)``

    Read data (e.g. a matrix) from a DataSet in a HDF5 file. The same overloads as for dump are available.

For example:

.. code-block:: cpp

    #include <xtensor/xio.hpp>
    #include <xtensor-io/xhighfive.hpp>

    int main()
    {
        xt::xtensor<double,2> A = xt::ones<double>({10,5});

        xt::dump_hdf5("example.h5", "/path/to/data", A);

        A = xt::load_hdf5<xt::xtensor<double,2>>("example.h5", "/path/to/data");

        std::cout << A << std::endl;

        return 0;
    }

Advanced interface
------------------

The advanced interface provides simple free-functions that write to or read from an open ``HighFive::File``. It consists of the following functions:

*   ``xt::dump(file, path, data[, xt::dump_mode])``

    Dump data (e.g. a matrix) to a DataSet in a HDF5 file. The data can be: ``scalar`` (incl. ``std::string``), ``std::vector<scalar>`` (incl. ``std::string``), ``xt::xarray<scalar>``, and ``xt::xtensor<scalar,dim>``.

*   ``xt::dump(file, path, data, {i,...})``

    Dump scalar to the index ``{i,...}`` of a DataSet. If the DataSet does not yet exist, an extendible DataSet of the appropriate rank and shape is created. If it does exists, the DataSet is resized if necessary.

*   ``data = xt::load<...>(file, path)``

    Read data (e.g. a matrix) from a DataSet in a HDF5 file. The same overloads as for dump are available.

*   ``data = xt::load<...>(file, path, {i,...})``

    Read scalar as index ``{i,...}`` of a DataSet.

For example:

.. code-block:: cpp

    #include <xtensor/xio.hpp>
    #include <xtensor-io/xhighfive.hpp>

    int main()
    {
        HighFive::File file("example.h5", HighFive::File::Overwrite);

        xt::xtensor<double,2> A = xt::ones<double>({10,5});

        xt::dump(file, "/path/to/data", A);

        xt::dump(file, "/path/to/data", A, xt::dump_mode::overwrite);

        A = xt::load<xt::xtensor<double,2>>(file, "/path/to/data");

        std::cout << A << std::endl;

        return 0;
    }

Compiling & dependencies
------------------------

This library uses header only `HighFive <https://github.com/BlueBrain/HighFive>`_ library and the HDF5 library. Both should be available upon compiling and linking respectively.

Manual
^^^^^^

Compiling can then proceed through

.. code-block:: bash

    g++ -std=c++14 -lhdf5 main.cpp

    # manually set paths
    g++ -I... -L... -std=c++14 -lhdf5 main.cpp

Alternatively, HDF5 provides a wrapper command that sets the paths to the HDF5 library (not to HighFive):

.. code-block:: bash

    h5c++ -std=c++14 main.cpp

Using CMake
^^^^^^^^^^^

The following basic structure of ``CMakeLists.txt`` can be used:

.. code-block:: cmake

  cmake_minimum_required(VERSION 2.8.12)

  # define a project name
  project(example)

  # define empty list of libraries to link
  set(PROJECT_LIBS "")

  # enforce the C++ standard
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  # set optimization level
  set(CMAKE_BUILD_TYPE Release)

  # ...

  # find HighFive
  find_package(HighFive REQUIRED)

  # find HDF5
  find_package(HDF5 REQUIRED)
  include_directories(${HDF5_INCLUDE_DIRS})
  set(PROJECT_LIBS ${HDF5_C_LIBRARIES})

  # create executable
  add_executable(${PROJECT_NAME} main.cpp)

  # link libraries
  target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBS})




