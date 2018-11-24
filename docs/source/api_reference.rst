.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

API Reference
=============

NPZ files
---------

Defined in ``xtensor-io/xnpz.hpp``

.. doxygenfunction:: xt::load_npz(std::string)
   :project: xtensor-io

.. doxygenfunction:: xt::load_npz(std::string, std::string)
   :project: xtensor-io

.. doxygenfunction:: xt::dump_npz
   :project: xtensor-io

Images
------

Defined in ``xtensor-io/ximage.hpp``

.. doxygenfunction:: xt::load_image
   :project: xtensor-io

.. doxygenfunction:: xt::dump_image
   :project: xtensor-io

Audio files
-----------

Defined in ``xtensor-io/xaudio.hpp``

.. doxygenfunction:: xt::load_audio
   :project: xtensor-io

.. doxygenfunction:: xt::dump_audio
   :project: xtensor-io

HDF5 files
----------

Defined in ``xtensor-io/xhighfive.hpp``

.. doxygenfunction:: xt::dump_hdf5
   :project: xtensor-io

.. doxygenfunction:: xt::load_hdf5
   :project: xtensor-io

.. doxygenfunction:: xt::dump(HighFive::File&, const std::string&, const xt::xarray<T>&, xt::dump_mode)
   :project: xtensor-io

.. doxygenfunction:: xt::dump(HighFive::File&, const std::string&, const xt::xtensor<T, rank>&, xt::dump_mode)
   :project: xtensor-io

.. doxygenfunction:: xt::dump(HighFive::File&, const std::string&, const std::vector<T>&, xt::dump_mode)
   :project: xtensor-io

.. doxygenfunction:: xt::dump(HighFive::File&, const std::string&, const T&, xt::dump_mode)
   :project: xtensor-io

.. doxygenfunction:: xt::dump(HighFive::File&, const std::string&, const T&, const std::vector<std::size_t>&)
   :project: xtensor-io

.. doxygenfunction:: xt::load(const HighFive::File&, const std::string&, const std::vector<std::size_t>&)
   :project: xtensor-io

.. doxygenfunction:: xt::load(const HighFive::File&, const std::string&)
   :project: xtensor-io

.. doxygenfunction:: xt::extensions::exist(const HighFive::File&, const std::string&)
   :project: xtensor-io

.. doxygenfunction:: xt::extensions::create_group(const HighFive::File&, const std::string&)
   :project: xtensor-io

.. doxygenfunction:: xt::extensions::size(const HighFive::File&, const std::string&)
   :project: xtensor-io

.. doxygenfunction:: xt::extensions::shape(const HighFive::File&, const std::string&)
   :project: xtensor-io
