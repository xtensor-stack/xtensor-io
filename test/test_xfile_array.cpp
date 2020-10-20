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
#include "xtensor-io/xfile_array.hpp"
#include "xtensor-io/xio_disk_handler.hpp"

namespace xt
{
    TEST(xfile_array, indexed_access)
    {
        std::vector<size_t> shape = {2, 2, 2};
        xfile_array<double, xio_disk_handler<xcsv_config>> a;
        a.ignore_empty_path(true);
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
        auto a1 = xfile_array<double, xio_disk_handler<xcsv_config>>(broadcast(v1, {2, 2}), "a1");
        a1.ignore_empty_path(true);
        for (const auto& v: a1)
        {
            EXPECT_EQ(v, v1);
        }

        double v2 = 2. * v1;
        auto a2 = xfile_array<double, xio_disk_handler<xcsv_config>>(a1 + a1, "a2");
        a2.ignore_empty_path(true);
        for (const auto& v: a2)
        {
            EXPECT_EQ(v, v2);
        }

        a1.flush();
        a2.flush();

        std::ifstream in_file;
        in_file.open("a1");
        auto data = load_csv<double>(in_file);
        xarray<double> ref = {{v1, v1}, {v1, v1}};
        EXPECT_EQ(data, ref);
        in_file.close();

        in_file.open("a2");
        data = load_csv<double>(in_file);
        ref = {{v2, v2}, {v2, v2}};
        EXPECT_EQ(data, ref);
        in_file.close();
    }
}
