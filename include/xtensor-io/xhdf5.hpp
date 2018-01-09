/***************************************************************************
* Copyright (c) 2016, Wolf Vollprecht, Sylvain Corlay and Johan Mabille    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <vector>

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
        auto split_string(const std::string &str, char delim = '/')
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