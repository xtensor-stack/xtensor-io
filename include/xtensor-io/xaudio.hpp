#include <stdexcept>
#include <string>

#include <sndfile.hh>

#include "xtensor/xarray.hpp"
#include "xtensor/xeval.hpp"

namespace xt
{
    /**
     * Read a WAV file
     *
     * @param filename Open file with file name
     *
     * @return tuple with (samplerate, xarray holding the data)
     *
     * @tparam T select type
     */
    template <class T = short>
    auto load_wav(std::string filename)
    {
        SndfileHandle file(filename);
        // testing for format == 0 because file not exist isn't handled otherwise
        if (!file || file.format() == 0)
        {
            throw std::runtime_error(std::string("load_wav: ") + file.strError());
        }
        auto result = xarray<T>::from_shape({(std::size_t) file.frames(), (std::size_t) file.channels()});
        file.read(result.raw_data(), (sf_count_t) result.size());
        return std::make_tuple(file.samplerate(), std::move(result));
    }

    /**
     * Save an xarray in WAV sound file format
     *
     * @param filename save under filename
     * @param e xarray/xexpression
     * @param format select format (see sndfile documentation). Combine flags such as SF_FORMAT_WAV | SF_FORMAT_PCM_16
     * @param samplerate The samplerate
     */
    template <class E>
    void dump_wav(std::string filename, const xexpression<E>& e, int format, int samplerate)
    {
        auto&& de = xt::eval(e.derived_cast());
        SndfileHandle file(filename, SFM_WRITE, format, (int) de.shape()[1], samplerate);
        // need to explicitly check the rawHandle otherwise permission errors etc. are not detected
        if (!file || file.rawHandle() == nullptr)
        {
            throw std::runtime_error(std::string("dump_wav: ") + file.strError());
        }
        file.write(de.raw_data(), (sf_count_t) de.size());
    }
}