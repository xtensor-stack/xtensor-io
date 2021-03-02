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
#include "xtensor-io/xio_zlib.hpp"

namespace xt
{
    TEST(xzlib, load)
    {
        // file "test.zl" was created from Python with the following code:
        //
        // import zlib
        // import numpy as np
        // a = np.array([3, 2, 1, 0], dtype='float64')
        // b = a.tobytes()
        // c = zlib.compress(b, level=1)
        // with open('test.zl', 'wb') as f:
        //     f.write(c)
        using dtype = double;
        auto a0 = load_zlib<dtype>("files/test.zl");
        xarray<dtype> a1 = {3, 2, 1, 0};
        EXPECT_TRUE(xt::all(xt::equal(a0, a1)));
    }

    TEST(xzlib, save_load)
    {
        using dtype = double;
        xarray<dtype> a1 = {0, 1, 2, 3};
        dump_zlib("a1.zl", a1);
        auto a2 = load_zlib<dtype>("a1.zl");
        EXPECT_TRUE(xt::all(xt::equal(a1, a2)));
    }
}
