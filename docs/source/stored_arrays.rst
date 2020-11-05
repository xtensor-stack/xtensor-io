.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Stored Arrays
=============

Arrays can be stored on a file system using ``xfile_array``, enabling
persistence of data. This type of array is a file-backed cached ``xarray``,
meaning that you can use it as a normal array, and it will be flushed to the
file when it is destroyed or when ``flush()`` is explicitly called (provided
that its content has changed). Various file systems can be used, e.g. the local
file system or Google Cloud Storage, and data can be stored in various formats,
e.g. GZip or Blosc.

File Mode
---------

A file array can be created using one of the three following file modes:

- ``load``: the array is loaded from the file, meaning that the file must
  already exist, otherwise an exception is thrown.
- ``init``: the array will initialize the file, meaning that its content will
  be flushed regardless of any pre-existing file.
- ``init_on_fail``: the array is loaded from the file if it exists, otherwise
  the array will initialize the file. An initialization value can be used to
  fill the array.

The default mode is ``load``.

Example : on-disk file array
----------------------------


.. code:: cpp

    #include <xtensor-io/xfile_array.hpp>
    #include <xtensor-io/xio_binary.hpp>
    #include <xtensor-io/xio_disk_handler.hpp>

    int main()
    {
        // an on-disk file array stored in binary format
        using file_array = xt::xfile_array<double, xt::xio_disk_handler<xt::xio_binary_config>>;
        // since the file doesn't alreay exist, we use the "init" file mode
        file_array a1("a1.bin", xt::xfile_mode::init);

        std::vector<size_t> shape = {2, 2};
        a1.resize(shape);

        a1(0, 1) = 1.;
        // the in-memory value is changed, but not the on-disk file yet.
        // the on-disk file will change when the array is explicitly flushed,
        // or when it is destroyed (e.g. when going out of scope)

        a1.flush();
        // now the on-disk file has changed

        // a2 points to a1's file, we use the "load" file mode
        file_array a2("a1.bin", xt::xfile_mode::load);
        // the binary format doesn't store the shape
        a2.resize(shape);

        // a1 and a2 are equal
        assert(xt:all(xt::equal(a1, a2)));

        return 0;
    }

Stored Chunked Arrays
---------------------

As for a "normal" array, a chunked array can be stored on a file system. Under
the hood, it will use ``xfile_array`` to store the chunks. But rather than
having one file array for each chunk (which could have a huge memory footprint),
only a limited number of file arrays are used at the same time in a chunk pool.
The container which is responsible for managing the chunk pool (i.e. map
logical chunks in the array to physical chunks in the pool) is the
``xchunk_store_manager``, but you should not use it directly. Instead, we
provide factory functions to create a stored chunked array, as shown below:

.. code-block:: cpp

    #include "xtensor-io/xchunk_store_manager.hpp"
    #include "xtensor-io/xio_binary.hpp"
    #include "xtensor-io/xio_disk_handler.hpp"

    int main()
    {
        namespace fs = ghc::filesystem;

        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        std::string chunk_dir = "chunks1";
        fs::create_directory(chunk_dir);
        double init_value = 5.5;
        std::size_t pool_size = 2;  // a maximum of 2 chunks will be hold in memory

        auto a1 = xt::chunked_file_array<double, xt::xio_disk_handler<xt::xio_binary_config>>(shape, chunk_shape, chunk_dir, init_value, pool_size);

        a1(2, 1) = 1.2;  // this assigns to chunk (1, 0) in memory
        a1(1, 2) = 3.4;  // this assigns to chunk (0, 1) in memory
        a1(0, 0) = 5.6;  // because the pool is full, this saves chunk (1, 0) to disk
                         // and assigns to chunk (0, 0) in memory
        // when a1 is destroyed, all the modified chunks are saved to disk
        // this can be forced with a1.chunks().flush()
    }
