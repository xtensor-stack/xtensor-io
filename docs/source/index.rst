.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

.. image:: xtensor-io.svg

Input/Output routines for reading images, audio, and NumPy ``npz`` files for the xtensor_ C++ multi-dimensional array library.

Introduction
------------

xtensor-io offers bindings to popular open source libraries for reading and writing

- Images with OpenImageIO (many supported formats, among others: jpg, png, gif, tiff, bmp ...)
- Sound files with libsndfile (supports wav, ogg files)
- NumPy compressed file format (``npz``). Note: support for uncompressed NumPy files (``npy``) is available in core xtensor.
- Geospatial rasters with GDAL (many supported formats_)

Enabling xtensor-io in your C++ libraries
-----------------------------------------

``xtensor`` and ``xtensor-io`` require a modern C++ compiler supporting C++14. The following C++ compilers are supported:

- On Windows platforms, Visual C++ 2015 Update 2, or more recent
- On Unix platforms, gcc 4.9 or a recent version of Clang

Licensing
---------

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the LICENSE file for details.

.. toctree::
   :caption: INSTALLATION
   :maxdepth: 2

   installation

.. toctree::
   :caption: USAGE
   :maxdepth: 2

   basic_usage
   xhighfive

.. toctree::
   :caption: API REFERENCE
   :maxdepth: 2

   api_reference

.. toctree::
   :caption: DEVELOPER ZONE

   releasing

.. _`numpy to xtensor cheat sheet`: http://xtensor.readthedocs.io/en/latest/numpy.html
.. _xtensor: https://github.com/QuantStack/xtensor
.. _formats: https://gdal.org/drivers/raster/index.html
