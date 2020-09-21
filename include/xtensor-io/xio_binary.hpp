/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_BINARY_HPP
#define XTENSOR_IO_BINARY_HPP

#include <fstream>

#include "xtensor/xadapt.hpp"

namespace xt
{
    namespace detail
    {
        bool is_big_endian(void)
        {
            union {
                uint32_t i;
                char c[4];
            } bint = {0x01020304};

            return bint.c[0] == 1;
        }

        template <class T>
        void swap_endianness(const char* in_buffer, char* out_buffer, std::size_t size, T = T())
        {
            for (std::size_t i = 0; i < size; i++)
            {
                T tmp = *reinterpret_cast<const T*>(&(in_buffer[i * sizeof(T)]));
                for (std::size_t j = 0; j < sizeof(T); j++)
                {
                    out_buffer[i * sizeof(T) + j] = reinterpret_cast<char*>(&tmp)[(i + 1) * sizeof(T) - j - 1];
                }
            }
        }

        template <typename T>
        inline xt::svector<T> load_bin_file(std::istream& stream, bool as_big_endian)
        {
            stream.seekg(0, stream.end);
            auto uncompressed_size = static_cast<std::size_t>(stream.tellg());
            stream.seekg(0, stream.beg);
            std::size_t size = uncompressed_size / sizeof(T);
            xt::svector<T> uncompressed_buffer(size);
            char* buffer = reinterpret_cast<char*>(uncompressed_buffer.data());
            stream.read(buffer, (std::streamsize)uncompressed_size);
            if ((sizeof(T) > 1) && (as_big_endian != is_big_endian()))
            {
                swap_endianness<T>(buffer, buffer, size);
            }
            return uncompressed_buffer;
        }

        template <class O, class E>
        inline void dump_bin_stream(O& stream, const xexpression<E>& e, bool as_big_endian)
        {
            using value_type = typename E::value_type;
            const E& ex = e.derived_cast();
            auto&& eval_ex = eval(ex);
            auto shape = eval_ex.shape();
            std::size_t size = compute_size(shape);
            std::size_t uncompressed_size = size * sizeof(value_type);
            const char* uncompressed_buffer = reinterpret_cast<const char*>(eval_ex.data());
            if ((sizeof(value_type) > 1) && (as_big_endian != is_big_endian()))
            {
                char* swapped_buffer = new char[size * sizeof(value_type)];
                swap_endianness<value_type>(uncompressed_buffer, swapped_buffer, size);
                stream.write(swapped_buffer, std::streamsize(uncompressed_size));
                stream.flush();
                delete[] swapped_buffer;
            }
            else
            {
                stream.write(uncompressed_buffer, std::streamsize(uncompressed_size));
                stream.flush();
            }
        }
    }  // namespace detail

    /**
     * Save xexpression to binary format
     *
     * @param stream An output stream to which to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_bin(std::ostream& stream, const xexpression<E>& e, bool as_big_endian)
    {
        detail::dump_bin_stream(stream, e, as_big_endian);
    }

    /**
     * Save xexpression to binary format
     *
     * @param filename The filename or path to dump the data
     * @param e the xexpression
     */
    template <typename E>
    inline void dump_bin(const std::string& filename, const xexpression<E>& e, bool as_big_endian)
    {
        std::ofstream stream(filename, std::ofstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error("IO Error: failed to open file");
        }
        detail::dump_bin_stream(stream, e, as_big_endian);
    }

    /**
     * Save xexpression to binary format in a string
     *
     * @param e the xexpression
     */
    template <typename E>
    inline std::string dump_bin(const xexpression<E>& e, bool as_big_endian)
    {
        std::stringstream stream;
        detail::dump_bin_stream(stream, e, as_big_endian);
        return stream.str();
    }

    /**
     * Loads a binary file
     *
     * @param stream An input stream from which to load the file
     * @tparam T select the type of the binary file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from binary file
     */
    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_bin(std::istream& stream, bool as_big_endian)
    {
        xt::svector<T> uncompressed_buffer = detail::load_bin_file<T>(stream, as_big_endian);
        std::vector<std::size_t> shape = {uncompressed_buffer.size()};
        auto array = adapt(std::move(uncompressed_buffer), shape);
        return array;
    }

    /**
     * Loads a binary file
     *
     * @param filename The filename or path to the file
     * @tparam T select the type of the binary file
     * @tparam L select layout_type::column_major if you stored data in
     *           Fortran format
     * @return xarray with contents from binary file
     */
    template <typename T, layout_type L = layout_type::dynamic>
    inline auto load_bin(const std::string& filename, bool as_big_endian)
    {
        std::ifstream stream(filename, std::ifstream::binary);
        if (!stream.is_open())
        {
            std::runtime_error("load_bin: failed to open file " + filename);
        }
        return load_bin<T, L>(stream, as_big_endian);
    }

    struct xio_binary_config
    {
        const char* name;
        const char* version;
        bool big_endian;

        xio_binary_config()
            : name("binary")
            , version("1.0")
            , big_endian(false)
        {
        }

        template <class T>
        void write_to(T& j) const
        {
        }
    };

    template <class E>
    void load_file(std::istream& stream, xexpression<E>& e, const xio_binary_config& config)
    {
        E& ex = e.derived_cast();
        auto shape = ex.shape();
        ex = load_bin<typename E::value_type>(stream, config.big_endian);
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
    void dump_file(std::ostream& stream, const xexpression<E> &e, const xio_binary_config& config)
    {
        dump_bin(stream, e, config.big_endian);
    }
}  // namespace xt

#endif
