/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <cstdint>
#include <sstream>
#include <exception>

#include "gtest/gtest.h"
#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-io/xio_gcs_handler.hpp"

namespace xt
{
    TEST(xio_gcs_handler, read)
    {
        xio_gcs_handler<xio_gzip_config> h;
        xio_gcs_config c = {gcs::Client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials()))), "zarr-demo"};
        h.configure_io(c);
        xarray<int32_t> a0;
        h.read(a0, "v3/test.zr3/data/root/arthur/dent/c0/0");
        xarray<int32_t> a1 = {0, 1, 2, 3, 4, 10, 11, 12, 13, 14};
        EXPECT_TRUE(xt::all(xt::equal(a0, a1)));
    }
}
