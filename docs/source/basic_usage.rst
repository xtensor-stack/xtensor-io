.. Copyright (c) 2016, Wolf Vollprecht, Johan Mabille and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Basic Usage
===========

Example : Reading images, audio and NPZ files
---------------------------------------------


.. code:: cpp

    #include <iostream>
    #include <xtensor/xbuilder.hpp>
    #include <xtensor/xmath.hpp>
    #include <xtensor-io/xnpz.hpp>
    #include <xtensor-io/ximage.hpp>
    #include <xtensor-io/xaudio.hpp>

    int main()
    {
        // loads png image into xarray with shape HEIGHT x WIDTH x CHANNELS
        auto arr = xt::load_image("test.png");

        // write xarray out to JPEG image
        xt::dump_image("dumptest.jpg", arr + 5);

        // load npz file containing multiple arrays
        auto npz_map = xt::load_npz("test.npz");

        auto arr_0 = npz_map["arr_0"].cast<double>();
        auto arr_1 = npz_map["arr_1"].cast<unsigned long>();

        // open a wav file
        auto audio = xt::load_audio("files/xtensor.wav");
        std::cout << "Sampling Frequency: " << std::get<0>(audio) << std::endl;
        auto& arr = std::get<1>(audio);  // audio contents (like scipy.io.wavfile results)

        // save a sine wave sound
        int freq = 2000;
        int sampling_freq = 44100;
        double duration = 1.0;

        auto t = xt::arange(0.0, duration, 1.0 / sampling_freq);
        auto y = xt::sin(2.0 * numeric_constants<double>::PI * freq * t);

        xt::dump_audio("files/sine.wav", y, sampling_freq);

        return 0;
    }
