#include <stdexcept>
#include <string>
#include <OpenImageIO/imageio.h>

#include "xtensor/xarray.hpp"

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
    void dump_image(std::string filename, const xexpression<E>& e)
    {
        const E& ex = e.derived_cast();

        ImageOutput *out = ImageOutput::create(filename);
        if (!out)
        {
            // something went wrong
            throw std::runtime_error("Couldn't open file to write image.");
        }
        ImageSpec spec(ex.shape()[0], ex.shape()[1], ex.shape()[2], TypeDesc::UINT8);

        out->open(filename, spec);
        out->write_image (TypeDesc::UINT8, ex.raw_data());
        out->close();
        ImageOutput::destroy(out);
    }
}