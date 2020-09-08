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
#include "xtensor-io/xgzip.hpp"

namespace xt
{
    TEST(xgzip, load)
    {
        // file "test.gz" was created from Python with the following code:
        //
        // import gzip
        // import numpy as np
        // a = np.array([3, 2, 1, 0], dtype='float64')
        // b = a.tobytes()
        // with gzip.open('test.gz', 'wb') as f:
        //     f.write(b)
        using dtype = double;
        auto a0 = load_gzip<dtype>("files/test.gz");
        xarray<dtype> a1 = {3, 2, 1, 0};
        EXPECT_TRUE(xt::all(xt::equal(a0, a1)));
    }

    TEST(xgzip, save_load)
    {
        using dtype = double;
        xarray<dtype> a1 = {0, 1, 2, 3};
        dump_gzip("a1.gz", a1);
        auto a2 = load_gzip<dtype>("a1.gz");
        EXPECT_TRUE(xt::all(xt::equal(a1, a2)));
    }
}
