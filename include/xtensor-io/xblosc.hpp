/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_BLOSC_HPP
#define XTENSOR_IO_BLOSC_HPP

#include <fstream>

#include "xtensor/xadapt.hpp"
#include "blosc.h"

bool blosc_initialized = false;

namespace xt
{
    namespace detail
    {
        template <typename T>
        inline std::vector<T> load_blosc_file(std::istream& stream)
        {
            stream.seekg(0, stream.end);
            auto compressed_size = static_cast<std::size_t>(stream.tellg());
            stream.seekg(0, stream.beg);
            std::vector<char> compressed_buffer(compressed_size);
            stream.read(compressed_buffer.data(), (std::streamsize)compressed_size);
            std::size_t uncompressed_size = 0;
            int res = blosc_cbuffer_validate(compressed_buffer.data(), compressed_size, &uncompressed_size);
            if (res == -1)
            {
                throw std::runtime_error("unsupported file format version");
            }
            size_t ubuf_size = uncompressed_size / sizeof(T);
            if (uncompressed_size % sizeof(T) != size_t(0))
                ubuf_size += size_t(1);
            std::vector<T> uncompressed_buffer(ubuf_size);
            res = blosc_decompress(compressed_buffer.data(), uncompressed_buffer.data(), uncompressed_size);
            if (res <= 0)
            {
                throw std::runtime_error("unsupported file format version");
            }
            return uncompressed_buffer;
        }

        template <class O, class E>
        inline void dump_blosc_stream(O& stream, const xexpression<E>& e, int clevel, int doshuffle)
        {
            using value_type = typename E::value_type;
            const E& ex = e.derived_cast();
            auto&& eval_ex = eval(ex);
            auto shape = eval_ex.shape();
            std::size_t uncompressed_size = compute_size(shape) * sizeof(value_type);
            const char* uncompressed_buffer = reinterpret_cast<const char*>(eval_ex.data());
            std::size_t max_compressed_size = uncompressed_size + BLOSC_MAX_OVERHEAD;
            std::allocator<char> char_allocator;
            char* compressed_buffer = char_allocator.allocate(max_compressed_size);
            if (!blosc_initialized)
            {
                blosc_init();
                blosc_initialized = true;
            }
            int true_compressed_size = blosc_compress(clevel, doshuffle, sizeof(value_type), uncompressed_size, uncompressed_buffer, compressed_buffer, max_compressed_size);
            if (true_compressed_size == 0)
            {
                throw std::runtime_error("buffer is uncompressible");
            }
            else if (true_compressed_size < 0)
            {
                throw std::runtime_error("compression error");
            }
            stream.write(compressed_buffer,
                         std::streamsize(true_compressed_size));
            char_allocator.deallocate(compressed_buffer, max_compressed_size);
        }
    }  // namespace detail

    /**
     * Save xexpression to blosc format
     *
     * @param stream An output stream to which to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_blosc(std::ostream& stream, const xexpression<E>& e, int clevel=5, int doshuffle=1)
    {
        detail::dump_blosc_stream(stream, e, clevel, doshuffle);
    }

    /**
     * Save xexpression to blosc format
     *
     * @param filename The filename or path to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_blosc(const std::string& filename, const xexpression<E>& e, int clevel=5, int doshuffle=1)
    {
        std::ofstream stream(filename, std::ofstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error("IO Error: failed to open file");
        }
        detail::dump_blosc_stream(stream, e, clevel, doshuffle);
    }

    /**
     * Save xexpression to blosc format in a string
     *
     * @param e the xexpression
     */
    template <typename E>
    inline std::string dump_blosc(const xexpression<E>& e, int clevel=5, int doshuffle=1)
    {
        std::stringstream stream;
        detail::dump_blosc_stream(stream, e, clevel, doshuffle);
        return stream.str();
    }

    /**
     * Loads a blosc file
     *
     * @param stream An input stream from which to load the file
     * @tparam T select the type of the blosc file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from blosc file
     */
    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_blosc(std::istream& stream)
    {
        std::vector<T> uncompressed_buffer = detail::load_blosc_file<T>(stream);
        std::vector<std::size_t> shape = {uncompressed_buffer.size()};
        auto array = adapt(std::move(uncompressed_buffer), shape);
        return array;
    }

    /**
     * Loads a blosc file
     *
     * @param filename The filename or path to the file
     * @tparam T select the type of the blosc file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from blosc file
     */
    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_blosc(const std::string& filename)
    {
        std::ifstream stream(filename, std::ifstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error("load_blosc: failed to open file " + filename);
        }
        return load_blosc<T, L>(stream);
    }

    struct xblosc_config
    {
        const char* name;
        const char* version;
        int clevel;
        int doshuffle;

        xblosc_config()
            : name("blosc")
            , version(BLOSC_VERSION_STRING)
            , clevel(5)
            , doshuffle(1)
        {
        }
    };

    template <class E>
    void load_file(std::istream& stream, xexpression<E>& e, const xblosc_config&)
    {
        E& ex = e.derived_cast();
        auto shape = ex.shape();
        ex = load_blosc<typename E::value_type>(stream);
        if (!shape.empty())
        {
            if (compute_size(shape) != ex.size())
            {
                XTENSOR_THROW(std::runtime_error, "load_file: size mismatch");
            }
            ex.reshape(shape);
        }
    }

    template <class E>
    void dump_file(std::ostream& stream, const xexpression<E> &e, const xblosc_config& config)
    {
        dump_blosc(stream, e, config.clevel, config.doshuffle);
    }
}  // namespace xt

#endif
