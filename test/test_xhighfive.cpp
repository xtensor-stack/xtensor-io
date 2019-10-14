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
#include "xtensor/xrandom.hpp"
#include "xtensor-io/xhighfive.hpp"

namespace xt
{
    TEST(xhighfive, scalar)
    {
        HighFive::File file("test.h5", HighFive::File::Overwrite);

        double a = 1.2345;
        int b = 12345;
        std::string c = "12345";

        xt::dump(file, "/path/to/a", a);
        xt::dump(file, "/path/to/b", b);
        xt::dump(file, "/path/to/c", c);

        double a_r = xt::load<double>(file, "/path/to/a");
        int b_r = xt::load<int>(file, "/path/to/b");
        std::string c_r = xt::load<std::string>(file, "/path/to/c");

        EXPECT_TRUE(a == a_r);
        EXPECT_TRUE(b == b_r);
        EXPECT_TRUE(c == c_r);
    }

    TEST(xhighfive, xtensor)
    {
        HighFive::File file("test.h5", HighFive::File::Overwrite);

        xt::xtensor<double,2> A = 100. * xt::random::randn<double>({20,5});
        xt::xtensor<int,2> B = A;

        xt::dump(file, "/path/to/A", A);
        xt::dump(file, "/path/to/B", B);

        xt::xtensor<double,2> A_r = xt::load<xt::xtensor<double,2>>(file, "/path/to/A");
        xt::xtensor<int,2> B_r = xt::load<xt::xtensor<int,2>>(file, "/path/to/B");

        EXPECT_TRUE(xt::allclose(A, A_r));
        EXPECT_TRUE(xt::all(xt::equal(B, B_r)));
    }

    TEST(xhighfive, xarray)
    {
        HighFive::File file("test.h5", HighFive::File::Overwrite);

        xt::xarray<double> A = 100. * xt::random::randn<double>({20,5});
        xt::xarray<int> B = A;

        xt::dump(file, "/path/to/A", A);
        xt::dump(file, "/path/to/B", B);

        xt::xarray<double> A_r = xt::load<xt::xarray<double>>(file, "/path/to/A");
        xt::xarray<int> B_r = xt::load<xt::xarray<int>>(file, "/path/to/B");

        EXPECT_TRUE(xt::allclose(A, A_r));
        EXPECT_TRUE(xt::all(xt::equal(B, B_r)));
    }

    TEST(xhighfive, extend1d)
    {
        HighFive::File file("test.h5", HighFive::File::Overwrite);

        for (std::size_t i = 0; i < 10; ++i)
        {
            xt::dump(file, "/path/to/A", i, {i});
        }

        xt::xarray<std::size_t> A = xt::arange<std::size_t>(10);

        xt::xarray<std::size_t> A_r = xt::load<xt::xarray<std::size_t>>(file, "/path/to/A");

        std::size_t Amax = xt::load<std::size_t>(file, "/path/to/A", {9});

        EXPECT_TRUE(xt::allclose(A, A_r));
        EXPECT_TRUE(Amax == 9);
    }

    TEST(xhighfive, extend2d)
    {
        HighFive::File file("test.h5", HighFive::File::Overwrite);

        for (std::size_t i = 0; i < 10; ++i)
        {
            for (std::size_t j = 0; j < 5; ++j)
            {
                xt::dump(file, "/path/to/A", i*5+j, {i,j});
            }
        }

        xt::xarray<std::size_t> A = xt::arange<std::size_t>(10*5);

        A.reshape({10,5});

        xt::xarray<std::size_t> A_r = xt::load<xt::xarray<std::size_t>>(file, "/path/to/A");

        std::size_t Amax = xt::load<std::size_t>(file, "/path/to/A", {9,4});

        EXPECT_TRUE(xt::allclose(A, A_r));
        EXPECT_TRUE(Amax == 49);
    }

    TEST(xhighfive, files)
    {
        double scalar_double = 12.345;
        int scalar_int = 12345;
        std::string scalar_string = "12345";

        std::vector<double> vector_double = {1.1, 2.2, 3.3, 4.4, 5.5};
        std::vector<int> vector_int = {1, 2, 3, 4, 5};

        xt::xtensor<double,2> matrix_double = {{1.1, 2.2}, {3.3, 4.4}, {5.5, 6.6}};
        xt::xtensor<int,2> matrix_int = {{1, 2}, {3, 4}, {5, 6}};

        double scalar_double_r = xt::load_hdf5<double>("files/archive.h5", "/scalar/double");
        int scalar_int_r = xt::load_hdf5<int>("files/archive.h5", "/scalar/int");
        std::string scalar_string_r = xt::load_hdf5<std::string>("files/archive.h5", "/scalar/string");

        std::vector<double> vector_double_r = xt::load_hdf5<std::vector<double>>("files/archive.h5", "/vector/double");
        std::vector<int> vector_int_r = xt::load_hdf5<std::vector<int>>("files/archive.h5", "/vector/int");

        xt::xtensor<double,2> matrix_double_r = xt::load_hdf5<xt::xtensor<double,2>>("files/archive.h5", "/matrix/double");
        xt::xtensor<int,2> matrix_int_r = xt::load_hdf5<xt::xtensor<int,2>>("files/archive.h5", "/matrix/int");

        EXPECT_TRUE(scalar_double == scalar_double_r);
        EXPECT_TRUE(scalar_int == scalar_int_r);
        EXPECT_TRUE(scalar_string == scalar_string_r);

        EXPECT_TRUE(vector_double == vector_double_r);
        EXPECT_TRUE(vector_int == vector_int_r);

        EXPECT_TRUE(xt::allclose(matrix_double, matrix_double_r));
        EXPECT_TRUE(xt::allclose(matrix_int, matrix_int_r));
    }
}
