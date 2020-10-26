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
#include "xtensor-io/xfile_array.hpp"
#include <xtensor-io/xio_binary.hpp>
#include "xtensor-io/xio_disk_handler.hpp"

namespace xt
{
    TEST(xfile_array, indexed_access)
    {
        std::vector<size_t> shape = {2, 2, 2};
        xfile_array<double, xio_disk_handler<xio_binary_config>> a("a", xfile_mode::init);
        a.resize(shape);
        double val = 3.;
        for (auto it: a)
            it = val;
        for (auto it: a)
            ASSERT_EQ(it, val);
    }

    TEST(xfile_array, assign_expression)
    {
        double v1 = 3.;
        auto a1 = xfile_array<double, xio_disk_handler<xio_binary_config>>(broadcast(v1, {2, 2}), "a1");
        for (const auto& v: a1)
        {
            EXPECT_EQ(v, v1);
        }

        double v2 = 2. * v1;
        auto a2 = xfile_array<double, xio_disk_handler<xio_binary_config>>(a1 + a1, "a2");
        for (const auto& v: a2)
        {
            EXPECT_EQ(v, v2);
        }

        a1.flush();
        a2.flush();

        std::ifstream in_file;
        in_file.open("a1");
        auto data = load_bin<double>(in_file);
        xarray<double> ref = {v1, v1, v1, v1};
        EXPECT_TRUE(xt::all(xt::equal(data, ref)));
        in_file.close();

        in_file.open("a2");
        data = load_bin<double>(in_file);
        ref = {v2, v2, v2, v2};
        EXPECT_TRUE(xt::all(xt::equal(data, ref)));
        in_file.close();
    }

    TEST(xfile_array, flush)
    {
        std::vector<std::size_t> shape = {2, 2};
        auto a1 = xfile_array<double, xio_disk_handler<xio_binary_config>>(broadcast(5., shape), "a1");
        a1.flush();
        auto a2 = xfile_array<double, xio_disk_handler<xio_binary_config>>("");
        // point a2 to a1's file
        a2.set_path("a1");
        // they should have the same size
        EXPECT_EQ(a2.size(), compute_size(shape));
        // resize a2
        std::vector<std::size_t> new_shape = {3, 3};
        a2.resize(new_shape);
        // should not be flushed, as binary format only stores data (not shape)
        a2.flush();
        auto a3 = xfile_array<double, xio_disk_handler<xio_binary_config>>("");
        // point a3 to a1's file (which is also a2's file)
        a3.set_path("a1");
        // shape should not have been changed
        EXPECT_EQ(a3.size(), compute_size(shape));
    }
}
