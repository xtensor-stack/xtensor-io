/***************************************************************************
* Copyright (c) 2016, Wolf Vollprecht, Sylvain Corlay and Johan Mabille    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_XIMAGE_HPP
#define XTENSOR_IO_XIMAGE_HPP

#include <stdexcept>
#include <string>

#include "xtensor/xarray.hpp"
#include "xtensor/xeval.hpp"

#include "xtensor_io_config.hpp"

#ifdef __CLING__
    #pragma clang diagnostic push
    // silencing warnings from OpenEXR 2.2.0 / OpenImageIO
    #pragma clang diagnostic ignored "-Wdeprecated-register"
    #pragma cling load("OpenImageIO")
#endif

#include <OpenImageIO/imageio.h>

#ifdef __CLING__
    #pragma clang diagnostic pop
#endif

namespace xt
{
    /**
     * Load an image from file at filename.
     * Storage format is deduced from file ending.
     *
     * @param filename The path of the file to load
     *
     * @return xarray with image contents. The shape of the xarray
     *         is ``WIDTH x HEIGHT x CHANNELS`` of the loaded image, where
     *         ``CHANNELS`` are ordered in standard ``R G B (A)``.
     */
    template <class T = unsigned char>
    xarray<T> load_image(std::string filename)
    {
        // TODO handle types better
        OIIO::ImageInput* in = OIIO::ImageInput::open(filename);
        if (!in)
        {
            // something went wrong
            throw std::runtime_error("Error reading image.");
        }
        const OIIO::ImageSpec& spec = in->spec();
        int xres = spec.width;
        int yres = spec.height;
        int channels = spec.nchannels;

        // allocate memory
        auto image = xarray<T>::from_shape({static_cast<std::size_t>(spec.height),
                                            static_cast<std::size_t>(spec.width),
                                            static_cast<std::size_t>(spec.nchannels)});

        in->read_image(OIIO::TypeDesc::UINT8, image.raw_data());
        in->close();
        OIIO::ImageInput::destroy(in);

        return image;
    }


    /**
     * Save image to disk.
     * The desired image format is deduced from ``filename``.
     * Supported formats are those supported by OpenImageIO.
     * Most common formats are supported (jpg, png, gif, bmp, tiff).
     *
     * @param filename The path to the desired file
     * @param data Image data
     * @param quality If saving in a compressed format such as JPEG, setting the quality
     *                will select how strong the compression will reduce the image quality.
     *                Quality is an integer from 100 (best image quality, less compression)
     *                to 0 (worst image quality, low filesize).
     */
    template <class E>
    void dump_image(std::string filename, const xexpression<E>& data, int quality = 90)
    {
        auto&& ex = eval(data.derived_cast());

        OIIO::ImageOutput* out = OIIO::ImageOutput::create(filename);
        if (!out)
        {
            // something went wrong
            throw std::runtime_error("Error opening file to write image.");
        }
        OIIO::ImageSpec spec(static_cast<int>(ex.shape()[1]),
                             static_cast<int>(ex.shape()[0]),
                             static_cast<int>(ex.shape()[2]), OIIO::TypeDesc::UINT8);

        spec.attribute("CompressionQuality", quality);

        out->open(filename, spec);
        out->write_image(OIIO::TypeDesc::UINT8, ex.raw_data());
        out->close();
        OIIO::ImageOutput::destroy(out);
    }
}

#endif
