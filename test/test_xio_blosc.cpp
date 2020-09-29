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
#include "xtensor-io/xio_blosc.hpp"

namespace xt
{
    TEST(xblosc, load)
    {
        // file "test.blosc" was created from Python with the following code:
        //
        // import blosc
        // import numpy as np
        // a = np.array([3, 2, 1, 0], dtype='float64')
        // b = a.tobytes()
        // c = blosc.compress(b)
        // with open('test.blosc', 'wb') as f:
        //     f.write(c)
        using dtype = double;
        auto a0 = load_blosc<dtype>("files/test.blosc");
        xarray<dtype> a1 = {3, 2, 1, 0};
        EXPECT_TRUE(xt::all(xt::equal(a0, a1)));
    }

    TEST(xblosc, save_load)
    {
        using dtype = double;
        xarray<dtype> a1 = {0, 1, 2, 3};
        dump_blosc("a1.blosc", a1);
        auto a2 = load_blosc<dtype>("a1.blosc");
        EXPECT_TRUE(xt::all(xt::equal(a1, a2)));
    }
}
