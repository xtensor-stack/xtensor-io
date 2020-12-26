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
#include "xtensor-io/xio_vsilfile.hpp"
#include "xtensor-io/xio_binary.hpp"
#include "xtensor-io/xio_gdal_handler.hpp"

namespace xt
{
    TEST(xio_gdal_handler, write_read)
    {
        VSIInstallMemFileHandler();
        xio_gdal_handler<xio_binary_config> h;
        xarray<int32_t> a0 = {3, 2, 1, 4};
        h.write(a0, "/vsimem/test_file.tif", xfile_dirty(true));
        xarray<int32_t> a1;
        h.read(a1, "/vsimem/test_file.tif");
        EXPECT_TRUE(xt::all(xt::equal(a0, a1)));
    }
}
