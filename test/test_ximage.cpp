/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "gtest/gtest.h"
#include "xtensor/xoperation.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xio.hpp"

#include "xtensor-io/ximage.hpp"

namespace xt
{
    xarray<uint8_t> test_image_rgb = {{{  0,   0,   0},
                                       {255,   0,   0},
                                       {  0, 255,   0}},
                                      {{  0,   0, 255},
                                       {228, 135, 255},
                                       {255, 255, 255}}};

    xarray<uint8_t> test_image_rgba = {{{  0,   0,   0, 255},
                                        {255,   0,   0, 255},
                                        {  0, 255,   0, 255}},
                                       {{  0,   0, 255, 255},
                                        {228, 135, 255, 255},
                                        {255, 255, 255, 255}}};

    xarray<uint8_t> test_image_jpg = {{{  0,   0,  35},
                                       {255,  12,   0},
                                       {  0, 255,   0}},
                                      {{  5,  10, 236},
                                       {222, 124, 237},
                                       {255, 253, 255}}};

    TEST(ximage, load_png)
    {
        auto img = load_image("files/test.png");
        EXPECT_TRUE(all(equal(test_image_rgb, img)));
    }

    TEST(ximage, load_gif)
    {
        auto img = load_image("files/test.gif");
        EXPECT_TRUE(all(equal(test_image_rgba, img)));
    }

    TEST(ximage, load_jpg)
    {
        auto img = load_image("files/test.jpg");
        bool test = (amax(test_image_rgb - img)() <= 35);
        EXPECT_TRUE(test);
    }

    TEST(ximage, exceptions)
    {
        EXPECT_THROW(dump_image("files/dump_test.png", view(test_image_rgb, all(), 0, 0)), std::runtime_error);
        EXPECT_THROW(dump_image("files/dump_test.png", view(test_image_rgb, all(), all(), all(), newaxis())), std::runtime_error);
    }

    TEST(ximage, save_png)
    {
        {
            // save as 'unsigned char'
            dump_image("files/dump_test.png", test_image_rgb);
            auto img = load_image("files/dump_test.png");
            EXPECT_TRUE(all(equal(test_image_rgb, img)));
        }
        {
            // save as 'int'
            dump_image("files/dump_test.png", 1*test_image_rgb);
            auto img = load_image("files/dump_test.png");
            EXPECT_TRUE(all(equal(test_image_rgb, img)));
        }
    }

    TEST(ximage, save_gif)
    {
        dump_image("files/dump_test.gif", test_image_rgba);
        auto img = load_image("files/dump_test.gif");
        EXPECT_TRUE(all(equal(test_image_rgba, img)));
    }

    TEST(ximage, save_jpg)
    {
        auto img_large = load_image("files/big.jpg");
        dump_image("files/dump_test.jpg", img_large);
        auto img = load_image("files/dump_test.jpg");
        bool test = (amax(img - img_large)() <= 50);
        EXPECT_TRUE(test);
    }

    TEST(ximage, float_img)
    {
        xarray<float> test_float(test_image_rgb);

        // TIFF supports float pixels
        dump_image("files/dump_test_float.tif", test_float);
        auto img = load_image<float>("files/dump_test_float.tif");
        EXPECT_EQ(test_float, img);

        // PNG does not support float pixels => test automatic conversion
        dump_image("files/dump_test_float.png", test_float);
        auto cimg = load_image("files/dump_test_float.png");
        EXPECT_EQ(test_image_rgb, cimg);
        img = load_image<float>("files/dump_test_float.png");
        EXPECT_TRUE(allclose(test_float, 255.0*img));
    }
}
