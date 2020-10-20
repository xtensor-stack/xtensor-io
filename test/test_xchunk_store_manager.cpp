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
#include <xtensor/xcsv.hpp>
#include "xtensor-io/xchunk_store_manager.hpp"
#include "xtensor-io/xfile_array.hpp"
#include "xtensor-io/xio_disk_handler.hpp"

namespace xt
{
    namespace fs = ghc::filesystem;

    template <class S>
    inline auto make_test_chunked_array(const S& shape,
                                        const S& chunk_shape,
                                        const std::string& chunk_dir,
                                        size_t pool_size)
    {
        return chunked_file_array<double, xio_disk_handler<xcsv_config>>(shape, chunk_shape, chunk_dir, pool_size);
    }

    TEST(xchunked_array, disk_array)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        std::string chunk_dir = "files";
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
        xt::xarray<double> ref;
        xt::xarray<double> data;
        in_file.open(chunk_dir + "/1.0");
        data = xt::load_csv<double>(in_file);
        ref = {{0, v1}, {0, 0}};
        EXPECT_EQ(data, ref);
        in_file.close();

        a1.chunks().flush();
        in_file.open(chunk_dir + "/0.1");
        data = xt::load_csv<double>(in_file);
        ref = {{0, 0}, {v2, 0}};
        EXPECT_EQ(data, ref);
        in_file.close();

        in_file.open(chunk_dir + "/0.0");
        data = xt::load_csv<double>(in_file);
        ref = {{v3, 0}, {0, 0}};
        EXPECT_EQ(data, ref);
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
}
