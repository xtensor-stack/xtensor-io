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
#include "xtensor-io/xblosc.hpp"

namespace xt
{
    TEST(xblosc, save_load)
    {
        using dtype = double;
        xarray<dtype> a1 {0, 1, 2, 3};
        dump_blosc("a1_blosc", a1);
        auto a2 = load_blosc<dtype>("a1_blosc");
        EXPECT_TRUE(xt::all(xt::equal(a1, a2)));
    }
}
