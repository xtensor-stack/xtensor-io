/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "gtest/gtest.h"

#include <xtensor/xbroadcast.hpp>
#include "xtensor-io/xchunk_store_manager.hpp"
#include "xtensor-io/xfile_array.hpp"
#include <xtensor-io/xio_binary.hpp>
#include "xtensor-io/xio_disk_handler.hpp"

namespace xt
{
    namespace fs = ghc::filesystem;

    template <class S>
    inline auto make_test_chunked_array(const S& shape,
                                        const S& chunk_shape,
                                        const std::string& chunk_dir,
                                        size_t pool_size,
                                        bool init=false,
                                        double init_value=0)
    {
        if (init)
        {
            return chunked_file_array<double, xio_disk_handler<xio_binary_config>>(shape, chunk_shape, chunk_dir, init_value, pool_size);
        }
        else
        {
            return chunked_file_array<double, xio_disk_handler<xio_binary_config>>(shape, chunk_shape, chunk_dir, pool_size);
        }
    }

    template <class T>
    inline void check_file_equal(const std::string& path, const T&& ref)
    {
        std::ifstream in_file;
        in_file.open(path);
        xt::xarray<double> data;
        data = xt::load_bin<double>(in_file);
        EXPECT_EQ(data, ref);
        in_file.close();
    }

    TEST(xchunked_array, disk_array)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        std::string chunk_dir = "files0";
        fs::create_directory(chunk_dir);
        std::size_t pool_size = 2;
        auto a1 = make_test_chunked_array(shape, chunk_shape, chunk_dir, pool_size);
        std::vector<size_t> idx = {1, 2};
        double v1 = 3.4;
        double v2 = 5.6;
        double v3 = 7.8;
        a1(2, 1) = v1;
        a1[idx] = v2;
        a1(0, 0) = v3; // this should unload chunk 1.0
        EXPECT_EQ(a1(2, 1), v1);
        EXPECT_EQ(a1[idx], v2);
        EXPECT_EQ(a1(0, 0), v3);

        std::ifstream in_file;
        xt::xarray<double> data;
        in_file.open(chunk_dir + "/1.0");
        data = xt::load_bin<double>(in_file);
        EXPECT_EQ(data(1), v1);
        in_file.close();

        a1.chunks().flush();
        in_file.open(chunk_dir + "/0.1");
        data = xt::load_bin<double>(in_file);
        EXPECT_EQ(data(2), v2);
        in_file.close();

        in_file.open(chunk_dir + "/0.0");
        data = xt::load_bin<double>(in_file);
        EXPECT_EQ(data(0), v3);
        in_file.close();
    }

    TEST(xchunked_array, assign_disk_array)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        std::size_t pool_size = 2;
        std::string chunk_dir1 = "files1";
        std::string chunk_dir2 = "files2";
        fs::create_directory(chunk_dir1);
        fs::create_directory(chunk_dir2);

        auto a1 = make_test_chunked_array(shape, chunk_shape, chunk_dir1, pool_size);
        double v1[16u];
        double sv1 = 1.;
        std::generate(v1, v1+16u, [&sv1]() { return sv1++ + 0.5; });
        std::copy(v1, v1+16u, a1.begin());
        a1.chunks().flush();

        auto a2 = make_test_chunked_array(shape, chunk_shape, chunk_dir2, pool_size);
        double v2[16u];
        double sv2 = 1.;
        std::generate(v2, v2+16u, [&sv2]() { return sv2++ + 0.5; });
        std::copy(v2, v2+16u, a2.begin());

        a2 = a1;
        a2.chunks().flush();

        for (ptrdiff_t i = 0; i < 16; ++i)
        {
            EXPECT_EQ(a2.begin()[i], v1[i]);
        }
    }

    TEST(xchunked_array, init_value)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        std::string chunk_dir = "files3";
        fs::create_directory(chunk_dir);
        std::size_t pool_size = 1;
        double init_value = 5.5;
        auto a1 = make_test_chunked_array(shape, chunk_shape, chunk_dir, pool_size, true, init_value);
        for (auto v: a1)
        {
            EXPECT_EQ(v, init_value);
        }
    }

    TEST(xchunked_array, chunked_assign_chunked)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        std::string chunk_dir1 = "files4";
        fs::create_directory(chunk_dir1);
        auto a1 = chunked_file_array<double, xio_disk_handler<xio_binary_config>>(shape, chunk_shape, chunk_dir1);
        std::string chunk_dir2 = "files5";
        fs::create_directory(chunk_dir2);
        auto a2 = chunked_file_array<double, xio_disk_handler<xio_binary_config>>(shape, chunk_shape, chunk_dir2);
        auto a3 = arange(4 * 4).reshape({4, 4});
        noalias(a2) = a3;
        a2.chunks().flush();
        // check that a2 has correct chunks
        check_file_equal(chunk_dir2 + "/0.0", xt::xarray<double>({0, 1, 4, 5}));
        check_file_equal(chunk_dir2 + "/1.0", xt::xarray<double>({8, 9, 12, 13}));
        check_file_equal(chunk_dir2 + "/0.1", xt::xarray<double>({2, 3, 6, 7}));
        check_file_equal(chunk_dir2 + "/1.1", xt::xarray<double>({10, 11, 14, 15}));

        noalias(a1) = a2;
        a1.chunks().flush();
        // check that a1 has correct chunks
        check_file_equal(chunk_dir1 + "/0.0", xt::xarray<double>({0, 1, 4, 5}));
        check_file_equal(chunk_dir1 + "/1.0", xt::xarray<double>({8, 9, 12, 13}));
        check_file_equal(chunk_dir1 + "/0.1", xt::xarray<double>({2, 3, 6, 7}));
        check_file_equal(chunk_dir1 + "/1.1", xt::xarray<double>({10, 11, 14, 15}));
    }

    TEST(xchunked_array, shape_initializer_list)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        auto a = chunked_file_array<double, xio_disk_handler<xio_binary_config>>({4, 4}, {2, 2}, "files3");
        for (auto v: a)
        {
            EXPECT_EQ(v, 5.5);
        }
    }
}
