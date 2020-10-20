#ifndef XTENSOR_IO_GCS_HANDLER_HPP
#define XTENSOR_IO_GCS_HANDLER_HPP

#include "xtensor/xarray.hpp"
#include "xtensor/xexpression.hpp"
#include "google/cloud/storage/client.h"

namespace gcs = google::cloud::storage;

namespace xt
{
    template <class C>
    class xio_gcs_handler
    {
    public:

        xio_gcs_handler();

        template <class E>
        void write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty) const;

        template <class ET>
        void read(ET& array, const std::string& path, bool throw_on_fail = false) const;

        void configure_format(const C& format_config);
        void split_bucket_path(const std::string& path, std::string& bucket_name, std::string& file_path) const;

    private:

        C m_format_config;
    };

    template <class C>
    xio_gcs_handler<C>::xio_gcs_handler()
    {
    }

    template <class C>
    template <class E>
    inline void xio_gcs_handler<C>::write(const xexpression<E>& expression, const std::string& path, xfile_dirty dirty) const
    {
        if (m_format_config.will_dump(dirty))
        {
            std::string bucket_name;
            std::string file_path;
            split_bucket_path(path, bucket_name, file_path);
            gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
            auto writer = client.WriteObject(bucket_name, file_path);
            dump_file(writer, expression, m_format_config);
        }
    }

    template <class C>
    template <class ET>
    inline void xio_gcs_handler<C>::read(ET& array, const std::string& path, bool throw_on_fail) const
    {
        std::string bucket_name;
        std::string file_path;
        split_bucket_path(path, bucket_name, file_path);
        gcs::Client client((gcs::ClientOptions(gcs::oauth2::CreateAnonymousCredentials())));
        auto reader = client.ReadObject(bucket_name, file_path);
        load_file<ET>(reader, array, m_format_config);
    }

    template <class C>
    inline void xio_gcs_handler<C>::configure_format(const C& format_config)
    {
        m_format_config = format_config;
    }

    template <class C>
    inline void xio_gcs_handler<C>::split_bucket_path(const std::string& path, std::string& bucket_name, std::string& file_path) const
    {
        std::size_t i = path.find('/');
        bucket_name = path.substr(0, i);
        file_path = path.substr(i + 1);
    }

}

#endif
