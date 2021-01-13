/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "gtest/gtest.h"

#include <cpl_vsi.h>
#include "xtensor-io/xio_vsilfile_wrapper.hpp"
#include "xtensor-io/xio_binary.hpp"
#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-io/xio_gdal_handler.hpp"

namespace xt
{
    TEST(xio_gdal_handler, write_read_vsimem)
    {
        xio_gdal_handler<xio_binary_config> h;
        xarray<int32_t> a0 = {3, 2, 1, 4};
        h.write(a0, "/vsimem/test_file.tif", xfile_dirty(true));
        xarray<int32_t> a1;
        h.read(a1, "/vsimem/test_file.tif");
        EXPECT_TRUE(xt::all(xt::equal(a0, a1)));
    }

    TEST(xio_gdal_handler, read_vsigs)
    {
        xio_gdal_handler<xio_gzip_config> h;
        xarray<int32_t> a0;
        h.read(a0, "/vsigs/zarr-demo/v3/test.zr3/data/root/arthur/dent/c0/0");
        xarray<int32_t> a1 = {0, 1, 2, 3, 4, 10, 11, 12, 13, 14};
        EXPECT_TRUE(xt::all(xt::equal(a0, a1)));
    }

    TEST(xio_gdal_handler, read_vsicurl)
    {
        xio_gdal_handler<xio_binary_config> h;
        xarray<char> a0;
        h.read(a0, "/vsicurl/https://raw.githubusercontent.com/xtensor-stack/xtensor-io/master/LICENSE");
        std::string ref = "Copyright";
        std::string res = a0.data();
        EXPECT_EQ(ref, res.substr(0, ref.size()));
    }
}
