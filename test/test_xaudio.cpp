/***************************************************************************
* Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "gtest/gtest.h"
#include "xtensor-io/xaudio.hpp"
#include "xtensor/xio.hpp"

namespace xt
{
	TEST(xaudio, load)
	{
		auto res = xt::load_audio("files/xtensor.wav");
		EXPECT_EQ(std::get<0>(res), 44100); // bitrate

		auto& arr = std::get<1>(res);
		EXPECT_EQ(arr.shape()[0], 70656ul); // channels
		EXPECT_EQ(arr.shape()[1], 2ul); // channels

		// test begin / end values
		EXPECT_EQ(arr(0, 0), -157);
		EXPECT_EQ(arr(0, 1), -57);
		EXPECT_EQ(arr(70655, 0), 233);
		EXPECT_EQ(arr(70655, 1), 140);
	}

	TEST(xaudio, roundtrip)
	{
		auto load_one = xt::load_audio("files/xtensor.wav");
		auto& arr_one = std::get<1>(load_one);
		xt::dump_audio("files/out.wav", arr_one, std::get<0>(load_one));

		auto load_two = xt::load_audio("files/out.wav");
		EXPECT_EQ(std::get<0>(load_one), std::get<0>(load_two));
		EXPECT_EQ(std::get<1>(load_one), std::get<1>(load_two));
	}

	TEST(xaudio, dump_sine)
	{
		int freq = 2000;
		int sampling_freq = 44100;
		double duration = 1.0;

		auto t = xt::arange(0.0, duration, 1.0 / sampling_freq);
		auto y = xt::sin(2.0 * numeric_constants<double>::PI * freq * t);

		xt::dump_audio("files/sine.wav", y, sampling_freq);
		auto load_back = xt::load_audio("files/sine.wav");

		xt::xarray<short> expected = xt::cast<short>(xt::round(y * 32767.0));
		expected.reshape({44100, 1});

		EXPECT_TRUE(all(equal(std::get<1>(load_back), expected)));
		EXPECT_EQ(std::get<0>(load_back), sampling_freq);
	}

	TEST(xaudio, errors)
	{
		EXPECT_THROW(load_audio("files/big.jpg"), std::runtime_error);
		EXPECT_THROW(load_audio("hahahahahaha.jpg"), std::runtime_error);

		auto load_one = xt::load_audio("files/xtensor.wav");
		auto& arr_one = std::get<1>(load_one);
		EXPECT_THROW(xt::dump_audio("/out.wav", arr_one, std::get<0>(load_one)),
					 std::runtime_error);
	}
}