/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
* Copyright (c) Andrew Hardin                                              *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_XGDAL_HPP
#define XTENSOR_IO_XGDAL_HPP

#include <functional>
#include <map>

#include <xtensor/xtensor.hpp>

// We rely exclusively on the stable C interface to GDAL.
#include <gdal.h>

namespace xt
{

    /**
     * The GDAL IO module supports reading and writing arrays with arbitrary index order
     * (e.g. [band,row,col]). The component enumeration contains tags for each dimension.
     */
    enum class component
    {
        band,
        row,
        column
    };

    using layout = std::array<component, 3>;

    /**
     * Get a band sequential layout; index order = [band, row, column].
     */
    inline layout layout_band_sequential()
    {
        return {component::band, component::row, component::column};
    }

    /**
     * Get a band interleaved by yixel layout; index order = [row, column, band].
     */
    inline layout layout_band_interleaved_pixel()
    {
        return {component::row, component::column, component::band};
    }

    /**
     * Get a band interleaved by line layout; index order = [row, band, column].
     */
    inline layout layout_band_interleaved_line()
    {
        return {component::row, component::band, component::column};
    }

    namespace detail
    {

        /** A type trait for converting a template type to GDALDataType value.
         */
        template<typename T>
        struct to_gdal_type;
#define XT_GDAL_DEFINE_TYPE(from, to) \
          template<> struct to_gdal_type<from> { \
              static const GDALDataType value = GDALDataType::to; \
          };
        XT_GDAL_DEFINE_TYPE(int8_t, GDT_Byte)
        XT_GDAL_DEFINE_TYPE(uint8_t, GDT_Byte)
        XT_GDAL_DEFINE_TYPE(uint16_t, GDT_UInt16)
        XT_GDAL_DEFINE_TYPE(int16_t, GDT_Int16)
        XT_GDAL_DEFINE_TYPE(uint32_t, GDT_UInt32)
        XT_GDAL_DEFINE_TYPE(int32_t, GDT_Int32)
        XT_GDAL_DEFINE_TYPE(float, GDT_Float32)
        XT_GDAL_DEFINE_TYPE(double, GDT_Float64)
#undef XT_GDAL_DEFINE_TYPE


        /**
         * A utility structure that records the spacing of pixel values (in bytes).
         * Used to provide arguments to GDALRasterIO.
         */
        struct space
        {
            /**
             * From GDAL documentation:
             * The byte offset from the start of one pixel value to the start of the next pixel value within a scanline.
             * If defaulted (0) the size of the datatype eBufType is used.
             */
            GSpacing pixel;
            /**
             * From GDAL documentation:
             * The byte offset from the start of one scanline in pData to the start of the next.
             * If defaulted (0) the size of the datatype eBufType * nBufXSize is used.
             */
            GSpacing line;
            /**
             * From GDAL documentation:
             * The byte offset from the start of one bands data to the start of the next.
             * If defaulted (0) the value will be nLineSpace * nBufYSize implying band sequential organization of the data buffer.
             */
            GSpacing band;
        };


        /**
         * Check that the given layout is valid.
         * @param item the layout to check.
         * @return false in the layout has duplicate entries (e.g. {band, band, row}).
         */
        static bool valid_layout(layout item)
        {
            return
                    item[0] != item[1] &&
                    item[0] != item[2] &&
                    item[1] != item[2];
        }

        /**
         * Convert standard raster dimensions into an xt shape.
         * @param item the layout that should be converted into a shape.
         * @param band_count number of bands.
         * @param nx width of the raster.
         * @param ny height of the raster.
         * @return an xt shape.
         */
        std::array<size_t, 3> layout_as_shape(layout item, size_t band_count, size_t nx, size_t ny)
        {
            std::map<component, size_t> dimmap
                    {
                            {component::band,   band_count},
                            {component::row,    ny},
                            {component::column, nx}
                    };
            return {
                    dimmap[item[0]],
                    dimmap[item[1]],
                    dimmap[item[2]]
            };
        }

        /**
         * Take the standard raster dimensions and convert them into byte level strides for GDALRasterIO().
         * @param item the layout to be converted into spacing.
         * @param band_count number of bands.
         * @param nx width of the raster.
         * @param ny height of the raster.
         * @param pixel_byte_count number of bytes in one pixel.
         * @return spacing in bytes.
         */
        static space
        layout_as_space(layout item, GSpacing band_count, GSpacing nx, GSpacing ny, GSpacing pixel_byte_count)
        {

            // All 6 cases are enumerated.
            // Perhaps there's a cleaner way to write this method?
            space ans{};
            if (item[0] == component::band)
            {
                if (item[1] == component::row)
                {
                    // {b, y, x }
                    ans.band = nx * ny;
                    ans.line = nx;
                    ans.pixel = 1;
                }
                else
                {
                    // {b, x, y}
                    ans.band = nx * ny;
                    ans.line = 1;
                    ans.pixel = ny;
                }
            }
            else if (item[1] == component::band)
            {
                if (item[0] == component::row)
                {
                    // {y, b, x}
                    ans.pixel = 1;
                    ans.line = nx * band_count;
                    ans.band = nx;
                }
                else
                {
                    // {x, b, y}
                    ans.pixel = ny * band_count;
                    ans.line = 1;
                    ans.band = ny;
                }
            }
            else
            {
                if (item[0] == component::row)
                {
                    // {y, x, b}
                    ans.pixel = band_count;
                    ans.line = band_count * nx;
                    ans.band = 1;
                }
                else
                {
                    // {x, y, b}
                    ans.pixel = band_count * ny;
                    ans.line = band_count;
                    ans.band = 1;
                }
            }
            // Convert from pixel -> bytes.
            ans.pixel *= pixel_byte_count;
            ans.band *= pixel_byte_count;
            ans.line *= pixel_byte_count;
            return ans;
        }
    }  // namespace detail

    /**
     * Options for loading a GDAL dataset.
     */
    struct load_gdal_options
    {
        /**
         * Load every band and return band-sequential memory (index order = [band, row, column]).
         */
        load_gdal_options()
                : interleave(layout_band_sequential()),
                  bands_to_load(/*load every available band */),
                  error_handler([](const std::string &msg)
                                { throw std::runtime_error("load_gdal(): " + msg); })
        {}

        /**
         * The desired layout of the returned tensor - defaults to band sequential([band, row, col]).
         * Arbitrary index order is supported, like band interleaved by pixel ([row, col, band]).
         */
        layout interleave;

        /**
         * The indices of bands to read from the dataset. All bands are read if empty.
         * @remark Per GDAL convention, these indices start at @em one (not zero).
         */
        std::vector<int> bands_to_load;

        /**
         * The error handler used to report errors (e.g. file missing or a read fails).
         * By default, a std::runtime_error is thrown when an error is encountered
         */
        std::function<void(const std::string &)> error_handler;
    };


    /**
     * Load pixels from a GDAL dataset.
     * @tparam T the type of pixels to return. If it doesn't match the content of the raster,
     *           GDAL will silently cast to this data type (which may cause loss of precision).
     * @param file_path the file to open.
     * @param options options to use when loading.
     * @return pixels; band sequential by default, but index order is controlled by options.
     */
    template<typename T>
    xtensor<T, 3> load_gdal(const std::string &file_path, load_gdal_options options = {})
    {
        auto dataset = GDALOpen(file_path.c_str(), GDALAccess::GA_ReadOnly);
        if (!dataset)
        {
            options.error_handler("error opening GDAL dataset '" + file_path + "'.");
            return {};
        }
        auto ans = load_gdal < T > (dataset, std::move(options));
        GDALClose(dataset);
        return ans;
    }

    /**
     * Load pixels from an opened GDAL dataset.
     * @tparam T in type of pixels to return. If this doesn't match the content of the dataset,
     *           GDAL will silently cast to this data type (which may cause loss of precision).
     * @param dataset an opened GDAL dataset.
     * @param options options to used when loading.
     * @return pixels; band sequential by default, but index order is controlled by options.
     */
    template<typename T>
    xtensor<T, 3> load_gdal(GDALDatasetH dataset, load_gdal_options options = {})
    {
        // Check if the user provided bands. If not, we'll load every band in the dataset.
        auto &bands = options.bands_to_load;
        if (bands.empty())
        {
            int count = GDALGetRasterCount(dataset);
            bands.resize(static_cast<size_t>(count));
            std::iota(bands.begin(), bands.end(), 1);
        }

        // Pull dataset dimensions.
        // Need both signed (for GDAL) and unsigned (for XT).
        int band_count = static_cast<int>(bands.size());
        int nx = GDALGetRasterXSize(dataset);
        int ny = GDALGetRasterYSize(dataset);
        auto band_count_u = bands.size();
        auto nx_u = static_cast<size_t>(nx);
        auto ny_u = static_cast<size_t>(ny);

        // Setup the shape and pixel spacing based on the user provided layout.
        if (!detail::valid_layout(options.interleave))
        {
            options.error_handler("the given interleave option has duplicate entries");
            return {};
        }
        auto shape = detail::layout_as_shape(options.interleave, band_count_u, nx_u, ny_u);
        auto spacing = detail::layout_as_space(options.interleave, band_count, nx, ny, sizeof(T));
        xt::xtensor<T, 3> ans(shape);

        // Read from the dataset. Again, there are some hurdles related
        // to the arbitrary layout order.
        auto error = GDALDatasetRasterIOEx(
                dataset,
                GDALRWFlag::GF_Read,
                0, 0,
                nx, ny,
                ans.data(),
                nx, ny,
                detail::to_gdal_type<T>::value,
                band_count,
                bands.data(),
                spacing.pixel,
                spacing.line,
                spacing.band,
                nullptr
        );
        if (error != CPLErr::CE_None)
        {
            options.error_handler("failed to read from dataset");
            return {};
        }
        return ans;
    }
}  // namespace xt


#endif // XTENSOR_IO_XGDAL_HPP