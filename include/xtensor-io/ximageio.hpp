#include <stdexcept>
#include <string>

#include <OpenImageIO/imageio.h>

#include "xtensor/xarray.hpp"
#include "xtensor/xeval.hpp"

using namespace OIIO;

namespace xt
{
    // Handle types better ...
    template <class T = unsigned char>
    xarray<T> load_image(std::string filename)
    {
        ImageInput *in = ImageInput::open(filename);
        if (!in)
        {
            // something went wrong
            throw std::runtime_error("Couldn't read image");
        }
        const ImageSpec &spec = in->spec();
        int xres = spec.width;
        int yres = spec.height;
        int channels = spec.nchannels;

        // allocate memory
        auto image = xarray<T>::from_shape({static_cast<std::size_t>(spec.width),
                                            static_cast<std::size_t>(spec.height),
                                            static_cast<std::size_t>(spec.nchannels)});

        in->read_image(TypeDesc::UINT8, image.raw_data());
        in->close();
        ImageInput::destroy(in);

        return image;
    }

    template <class E>
    void dump_image(std::string filename, const xexpression<E>& e, int quality = 90)
    {
        auto&& ex = eval(e.derived_cast());

        ImageOutput *out = ImageOutput::create(filename);
        if (!out)
        {
            // something went wrong
            throw std::runtime_error("Couldn't open file to write image.");
        }
        ImageSpec spec((int) ex.shape()[0], (int) ex.shape()[1], (int) ex.shape()[2], TypeDesc::UINT8);

        spec.attribute("CompressionQuality", quality);

        out->open(filename, spec);
        out->write_image(TypeDesc::UINT8, ex.raw_data());
        out->close();
        ImageOutput::destroy(out);
    }
}