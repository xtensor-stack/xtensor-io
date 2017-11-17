.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.


.. raw:: html

   <style>
   .rst-content .section>img {
       width: 30px;
       margin-bottom: 0;
       margin-top: 0;
       margin-right: 15px;
       margin-left: 15px;
       float: left;
   }
   </style>

Installation
============

xtensor-io is a header-only library but depends on some traditional libraries that need to be installed.
On Linux, installation of the dependencies can be done through the package manager, anaconda or manual compilation.

.. image:: conda.svg

Using the conda package
-----------------------

A package for xtensor-io is available on the conda package manager.
The package will also pull all the dependencies (OpenImageIO, libsndfile and zlib).

.. code::

    conda install -c QuantStack xtensor-io

.. image:: cmake.svg

From source with cmake
----------------------

You can also install ``xtensor-io`` from source with cmake. On Unix platforms, from the source directory:
However, you need to make sure to have the required libraries available on your machine.
Note: you don't need all libraries if you only use parts of ``xtensor-io``. ``libsndfile`` is required for
xaudio, ``OpenImageIO`` for ximage and ``zlib`` for xnpz.

Installation of the dependencies on Linux:

.. code::

    # Ubuntu / Debian
    sudo apt-get install libsndfile-dev libopenimageio-dev zlib1g-dev
    # Fedora
    sudo dnf install libsndfile-devel OpenImageIO-devel zlib-devel


.. code::

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    make install

On Windows platforms, from the source directory:

.. code::

    mkdir build
    cd build
    cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    nmake
    nmake install
