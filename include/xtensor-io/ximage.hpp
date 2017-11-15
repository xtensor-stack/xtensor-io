#include <stdexcept>
#include <string>

#include <OpenImageIO/imageio.h>

#include "xtensor/xarray.hpp"
#include "xtensor/xeval.hpp"

namespace xt
{
    // Handle types better ...
    template <class T = unsigned char>
    xarray<T> load_image(std::string filename)
    {
        OIIO::ImageInput *in = OIIO::ImageInput::open(filename);
        if (!in)
        {
            // something went wrong
            throw std::runtime_error("Error reading image.");
        }
        const OIIO::ImageSpec &spec = in->spec();
        int xres = spec.width;
        int yres = spec.height;
        int channels = spec.nchannels;

        // allocate memory
        auto image = xarray<T>::from_shape({static_cast<std::size_t>(spec.width),
                                            static_cast<std::size_t>(spec.height),
                                            static_cast<std::size_t>(spec.nchannels)});

        in->read_image(OIIO::TypeDesc::UINT8, image.raw_data());
        in->close();
        OIIO::ImageInput::destroy(in);

        return image;
    }

    template <class E>
    void dump_image(std::string filename, const xexpression<E>& e, int quality = 90)
    {
        auto&& ex = eval(e.derived_cast());

        OIIO::ImageOutput *out = OIIO::ImageOutput::create(filename);
        if (!out)
        {
            // something went wrong
            throw std::runtime_error("Error opening file to write image.");
        }
        OIIO::ImageSpec spec((int) ex.shape()[0], (int) ex.shape()[1], (int) ex.shape()[2], OIIO::TypeDesc::UINT8);

        spec.attribute("CompressionQuality", quality);

        out->open(filename, spec);
        out->write_image(OIIO::TypeDesc::UINT8, ex.raw_data());
        out->close();
        OIIO::ImageOutput::destroy(out);
    }
}