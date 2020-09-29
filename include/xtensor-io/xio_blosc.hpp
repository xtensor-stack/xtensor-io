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
#include "xtensor-io.hpp"
#include "blosc.h"

namespace xt
{
    namespace detail
    {
        class xio_blosc
        {
        public:
            static void initialize()
            {
                if (!instance().m_initialized)
                {
                    blosc_init();
                    instance().m_initialized = true;
                }
            }

        private:
            xio_blosc()
            {
                m_initialized = false;
            }

            static xio_blosc& instance()
            {
                static xio_blosc instance;
                return instance;
            }
            bool m_initialized;
        };

        template <typename T>
        inline xt::svector<T> load_blosc_file(std::istream& stream, bool as_big_endian)
        {
            xio_blosc::initialize();
            stream.seekg(0, stream.end);
            auto compressed_size = static_cast<std::size_t>(stream.tellg());
            stream.seekg(0, stream.beg);
            xt::svector<char> compressed_buffer(compressed_size);
            stream.read(compressed_buffer.data(), (std::streamsize)compressed_size);
            std::size_t uncompressed_size = 0;
            int res = blosc_cbuffer_validate(compressed_buffer.data(), compressed_size, &uncompressed_size);
            if (res == -1)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: unsupported file format version");
            }
            size_t ubuf_size = uncompressed_size / sizeof(T);
            if (uncompressed_size % sizeof(T) != size_t(0))
                ubuf_size += size_t(1);
            xt::svector<T> uncompressed_buffer(ubuf_size);
            res = blosc_decompress(compressed_buffer.data(), uncompressed_buffer.data(), uncompressed_size);
            if (res <= 0)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: unsupported file format version");
            }
            if ((sizeof(T) > 1) && (as_big_endian != is_big_endian()))
            {
                swap_endianness(uncompressed_buffer);
            }
            return uncompressed_buffer;
        }

        template <class O, class E>
        inline void dump_blosc_stream(O& stream, const xexpression<E>& e, bool as_big_endian, int clevel, int shuffle, const char* cname, std::size_t blocksize)
        {
            xio_blosc::initialize();
            using value_type = typename E::value_type;
            const E& ex = e.derived_cast();
            auto&& eval_ex = eval(ex);
            auto shape = eval_ex.shape();
            std::size_t size = compute_size(shape);
            std::size_t uncompressed_size = size * sizeof(value_type);
            const char* uncompressed_buffer;
            xt::svector<value_type> swapped_buffer;
            if ((sizeof(value_type) > 1) && (as_big_endian != is_big_endian()))
            {
                swapped_buffer.resize(size);
                std::copy(eval_ex.data(), eval_ex.data() + size, swapped_buffer.begin());
                swap_endianness(swapped_buffer);
                uncompressed_buffer = reinterpret_cast<const char*>(swapped_buffer.data());
            }
            else
            {
                uncompressed_buffer = reinterpret_cast<const char*>(eval_ex.data());
            }
            std::size_t max_compressed_size = uncompressed_size + BLOSC_MAX_OVERHEAD;
            std::allocator<char> char_allocator;
            char* compressed_buffer = char_allocator.allocate(max_compressed_size);
            blosc_set_blocksize(blocksize);
            if (blosc_set_compressor(cname) == -1)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: compressor not supported (" + std::string(cname) + ")");
            }
            int true_compressed_size = blosc_compress(clevel, shuffle, sizeof(value_type), uncompressed_size, uncompressed_buffer, compressed_buffer, max_compressed_size);
            if (true_compressed_size == 0)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: buffer is uncompressible");
            }
            else if (true_compressed_size < 0)
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: compression error");
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
    inline void dump_blosc(std::ostream& stream, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        detail::dump_blosc_stream(stream, e, as_big_endian, clevel, shuffle, cname, blocksize);
    }

    /**
     * Save xexpression to blosc format
     *
     * @param filename The filename or path to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_blosc(const std::string& filename, const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        std::ofstream stream(filename, std::ofstream::binary);
        if (!stream.is_open())
        {
            XTENSOR_THROW(std::runtime_error, "Blosc: failed to open file " + filename);
        }
        detail::dump_blosc_stream(stream, e, as_big_endian, clevel, shuffle, cname, blocksize);
    }

    /**
     * Save xexpression to blosc format in a string
     *
     * @param e the xexpression
     */
    template <typename E>
    inline std::string dump_blosc(const xexpression<E>& e, bool as_big_endian=is_big_endian(), int clevel=5, int shuffle=1, const char* cname="blosclz", std::size_t blocksize=0)
    {
        std::stringstream stream;
        detail::dump_blosc_stream(stream, e, as_big_endian, clevel, shuffle, cname, blocksize);
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
    inline auto load_blosc(std::istream& stream, bool as_big_endian=is_big_endian())
    {
        xt::svector<T> uncompressed_buffer = detail::load_blosc_file<T>(stream, as_big_endian);
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
    inline auto load_blosc(const std::string& filename, bool as_big_endian=is_big_endian())
    {
        std::ifstream stream(filename, std::ifstream::binary);
        if (!stream.is_open())
        {
            XTENSOR_THROW(std::runtime_error, "Blosc: failed to open file " + filename);
        }
        return load_blosc<T, L>(stream, as_big_endian);
    }

    struct xio_blosc_config
    {
        const char* name;
        const char* version;
        bool big_endian;
        int clevel;
        int shuffle;
        const char* cname;
        const std::size_t blocksize;

        xio_blosc_config()
            : name("blosc")
            , version(BLOSC_VERSION_STRING)
            , big_endian(is_big_endian())
            , clevel(5)
            , shuffle(1)
            , cname("blosclz")
            , blocksize(0)
        {
        }

        template <class T>
        void write_to(T& j) const
        {
            j["clevel"] = clevel;
            j["shuffle"] = shuffle;
            j["cname"] = cname;
            j["blocksize"] = blocksize;
        }

        template <class T>
        void read_from(T& j)
        {
            clevel = j["clevel"];
            shuffle = j["shuffle"];
            cname = j["cname"];
            blocksize = j["blocksize"];
        }
    };

    template <class E>
    void load_file(std::istream& stream, xexpression<E>& e, const xio_blosc_config& config)
    {
        E& ex = e.derived_cast();
        auto shape = ex.shape();
        ex = load_blosc<typename E::value_type>(stream, config.big_endian);
        if (!shape.empty())
        {
            if (compute_size(shape) != ex.size())
            {
                XTENSOR_THROW(std::runtime_error, "Blosc: expected size (" + std::string(compute_size(shape)) + ") and actual size (" + std::string(ex.size()) + ") mismatch");
            }
            ex.reshape(shape);
        }
    }

    template <class E>
    void dump_file(std::ostream& stream, const xexpression<E> &e, const xio_blosc_config& config)
    {
        dump_blosc(stream, e, config.big_endian, config.clevel, config.shuffle, config.cname, config.blocksize);
    }
}  // namespace xt

#endif
