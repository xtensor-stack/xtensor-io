/***************************************************************************
* Copyright (c) 2016, Wolf Vollprecht, Sylvain Corlay and Johan Mabille    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <vector>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
namespace mmap {
    #include <sys/mman.h>
}
#include <unistd.h>
#include <fcntl.h>
#endif

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataType.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

#include "xtensor/xarray.hpp"
#include "xtensor/xtensor.hpp"
#include "xtensor/xadapt.hpp"
#include "xtensor/xeval.hpp"

namespace xt
{
    namespace HF = HighFive;

    namespace detail
    {
        auto split_string(const std::string& str, char delim = '/')
        {
            std::vector<std::string> cont;
            std::size_t current, previous = 0;
            current = str.find(delim);
            while (current != std::string::npos)
            {
                cont.push_back(str.substr(previous, current - previous));
                previous = current + 1;
                current = str.find(delim, previous);
            }
            cont.push_back(str.substr(previous, current - previous));
            return cont;
        }
    }

    /**
     * Mode selector for opening H5 File
     */
    enum H5Mode
    {
        ReadOnly = HF::File::ReadOnly,
        ReadWrite = HF::File::ReadWrite,
        Truncate = HF::File::Truncate,
        Excl = HF::File::Excl,
        Debug = HF::File::Debug,
        Create = HF::File::Create,
    };

    class x5file
    {
    public:

        x5file(const std::string& filename, const int mode)
            : m_file(filename, mode)
        {
        }

        /**
         * Cast HDF5 file dataset to xarray container.
         *
         * @param name path of the dataset
         * @tparam datatype to cast to
         */
        template <class T>
        auto cast(const std::string& name)
        {
            return cast_impl<xarray<T>>(name);
        }

        /**
         * Cast HDF5 file dataset to xtensor container.
         *
         * @param name path of the dataset
         * @tparam datatype to cast to
         * @tparam dim compile-time known dimensionality
         */
        template <class T, std::size_t dim>
        auto cast(const std::string& name)
        {
            return cast_impl<xtensor<T, dim>>(name);
        }

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
        /**
         * Memory Map HDF5 dataset to xadapt.
         *
         * @param name path of the dataset
         * @tparam datatype to cast to
         */
        template <class T>
        auto mmap(const std::string& name)
        {
            return mmap_impl<T, std::vector<std::size_t>>(name);
        }

        /**
         * Memory Map HDF5 dataset to xadapt.
         *
         * @param name path of the dataset
         * @tparam datatype to cast to
         * @tparam dim compile-time known dimensionality
         */
        template <class T, std::size_t dim>
        auto mmap(const std::string& name)
        {
            return mmap_impl<T, std::array<std::size_t, dim>>(name);
        }
#endif

        /**
         * Save an xexpression to the HDF5 file
         *
         * @param name path where the corresponding dataset will be created
         * @param data xexpression evaluating to the data to be saved
         */
        template <class E>
        void append(const std::string& name, const xexpression<E>& data)
        {
            auto&& d_data = xt::eval(data.derived_cast());
            HF::DataSpace space(std::vector<std::size_t>(d_data.shape().cbegin(), d_data.shape().cend()));
            HF::DataSet ds = m_file.createDataSet(name, space, HF::AtomicType<typename E::value_type>());
            ds.write(d_data.raw_data());
        }

        /**
         * @brief Create groups recursively
         * This function splits @em path at the "/" delimiter, and creates the necessary groups
         * recursively.
         *
         * @param path the path, delimited with "/"
         */
        void recursive_create_groups(const std::string& path)
        {
            auto split = detail::split_string(path);
            std::string gs;
            for (auto &el : split)
            {
                gs += "/" + el;
                std::cout << gs << std::endl;
                if (!m_file.exist(gs))
                {
                    m_file.createGroup(gs);
                }
            }
        }

        /**
         * Create a single group in the file context.
         */
        void create_group(const std::string& name)
        {
            m_file.createGroup(name);
        }


    private:

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
        template <class T, class ST>
        inline auto mmap_impl(const std::string& name)
        {
            HF::DataSet ds = m_file.getDataSet(name);
            std::size_t offset = ds.getOffset();

            int* fd;
            H5Fget_vfd_handle(m_file.getId(), H5P_DEFAULT, reinterpret_cast<void**>(&fd));

            std::size_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

            auto space = ds.getSpace();
            auto dims = space.getDimensions();
            auto sz = std::accumulate(dims.begin(), dims.end(), std::size_t(1), std::multiplies<>());

            std::size_t length = sz * sizeof(T);
            void* ptr = mmap::mmap(nullptr, length + offset - pa_offset, PROT_READ,
                                   MAP_PRIVATE, *fd, pa_offset);
            T* t_ptr = reinterpret_cast<T*>(ptr + offset - pa_offset);

            return xt::adapt(t_ptr, sz, xt::no_ownership(), ST(dims.begin(), dims.end()));
        }
#endif

        template <class T>
        inline T cast_impl(const std::string& name)
        {
            HF::DataSet ds = m_file.getDataSet(name);
            auto space = ds.getSpace();
            auto dims = space.getDimensions();
            T res = T::from_shape(dims);
            ds.read(res.raw_data());
            return res;
        }

        HF::File m_file;
    };

    /**
     * Open HDF5 file
     *
     * @param name location of the file in the filesystem
     */
    x5file open_hdf5(const std::string& name, const int mode = H5Mode::ReadWrite)
    {
        return x5file(name, mode);
    }
}