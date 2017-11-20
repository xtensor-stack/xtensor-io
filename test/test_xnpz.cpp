/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <cstdint>

#include "gtest/gtest.h"
#include "xtensor-io/xnpz.hpp"

namespace xt
{
    TEST(xnpz, load)
    {
        auto npz_map = xt::load_npz("files/uncompressed.npz");
        auto arr_0 = npz_map["arr_0"].cast<double>();
        auto arr_1 = npz_map["arr_1"].cast<uint64_t>(false);
        xt::xarray<uint64_t> xarr_1 = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
        EXPECT_TRUE(xt::all(xt::isclose(arr_0, linspace<double>(0, 100))));
        EXPECT_TRUE(xt::all(xt::equal(arr_1, xarr_1)));
    }

    TEST(xnpz, load_compressed)
    {
        auto npz_map = xt::load_npz("files/compressed.npz");
        auto arr_0 = npz_map["arr_0"].cast<uint64_t>(false);
        auto arr_1 = npz_map["arr_1"].cast<double>();
        xt::xarray<uint64_t> xarr_0 = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
        EXPECT_TRUE(xt::all(xt::isclose(arr_1, linspace<double>(0, 100))));
        EXPECT_TRUE(xt::all(xt::equal(arr_0, xarr_0)));
    }

    bool compare_binary_files(std::string fn1, std::string fn2, std::size_t n_zipped_files)
    {
        std::ifstream stream1(fn1, std::ios::in | std::ios::binary);
        std::vector<uint8_t> fn1_contents((std::istreambuf_iterator<char>(stream1)),
                                           std::istreambuf_iterator<char>());

        std::ifstream stream2(fn2, std::ios::in | std::ios::binary);
        std::vector<uint8_t> fn2_contents((std::istreambuf_iterator<char>(stream2)),
                                           std::istreambuf_iterator<char>());
        std::size_t unequal = 0;

        if (fn1_contents.size() != fn2_contents.size())
        {
            return false;
        }

        for (auto iter_fn1 = fn1_contents.begin(), iter_fn2 = fn2_contents.begin();
             iter_fn1 != fn1_contents.end();
             iter_fn1++, iter_fn2++)
        {
            if (*iter_fn1 != *iter_fn2)
            {
                unequal += 1;
            }
        }
        // this is date + time == 4 bytes per file + once in global header
        std::size_t unequal_allowed = (n_zipped_files + 2) * 3;
        if (unequal > unequal_allowed)
        {
            return false;
        }
    }

    TEST(xnpz, save_uncompressed)
    {
        dump_npz("files/dump_uncompressed.npz", "arr_0", linspace<double>(0, 100), false, false);
        xt::xarray<int64_t> arr = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
        dump_npz("files/dump_uncompressed.npz", "arr_1", arr);
        EXPECT_TRUE(compare_binary_files("files/dump_uncompressed.npz", "files/uncompressed.npz", 2));
    }

    TEST(xnpz, save_compressed)
    {
        xt::xarray<int64_t> arr = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
        dump_npz("files/dump_compressed.npz", "arr_1", linspace<double>(0, 100), true, false );
        dump_npz("files/dump_compressed.npz", "arr_0", arr, true);
        EXPECT_TRUE(compare_binary_files("files/dump_compressed.npz", "files/compressed.npz", 2));
    }
}