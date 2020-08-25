#include "gtest/gtest.h"
#include "xtensor-io/xgdal.hpp"
#include "xtensor/xadapt.hpp"
#include "xtensor/xoperation.hpp"

#include <gdal_frmts.h>

namespace xt
{
    // Verify our type traits (maybe just a floating point and integer one).
    static_assert(detail::to_gdal_type<float>::value == GDALDataType::GDT_Float32, "");
    static_assert(detail::to_gdal_type<int32_t>::value == GDALDataType::GDT_Int32, "");

    // Simple test fixture that creates an in-memory dataset with known dimensions and content.
    class xgdal_fixture : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Fill the buffer of pixel data.
            data.resize(static_cast<size_t>(bands * nx * ny));
            std::iota(data.begin(), data.end(), 1);

            // Create an in-memory TIFF dataset.
            GDALRegister_GTiff();
            auto driver = GDALGetDriverByName("GTiff");
            ASSERT_NE(driver, nullptr);
            auto ds = GDALCreate(driver, path.c_str(), nx, ny, bands, GDT_Int32, nullptr);
            ASSERT_NE(ds, nullptr);
            auto err = GDALDatasetRasterIO(ds, GDALRWFlag::GF_Write, 0, 0, nx, ny, data.data(), nx, ny,
                                           GDALDataType::GDT_Int32, bands, nullptr, 0, 0, 0);
            ASSERT_EQ(err, CPLErr::CE_None);
            GDALClose(ds);
        }

        std::string path = "/vsimem/test_file.tif";
        int bands = 2;
        int nx = 4;
        int ny = 3;
        size_t bands_u = static_cast<size_t>(bands);
        size_t nx_u = static_cast<size_t>(nx);
        size_t ny_u = static_cast<size_t>(ny);

        /*
         Very simple band sequential integers:

          1, 2, 3, 4,       (band 1)
          5, 6, 7, 8,
          9, 10, 11, 12,
          13, 14, 15, 16,   (band 2)
          17, 18, 19, 20,
          21, 22, 23, 24
        */
        std::vector<int> data;

    };

    TEST_F(xgdal_fixture, interleave)
    {
        // Check the six interleave options.
        auto check = [&](component a, component b, component c, const std::vector<int> &vtruth)
        {
            load_gdal_options options;
            options.interleave = {a, b, c};
            auto test = load_gdal<int>(path, options);
            auto truth = xt::adapt(vtruth, test.shape());
            EXPECT_TRUE(all(equal(truth, test)));
        };
        check(component::band, component::row, component::column,
              data);
        check(component::band, component::column, component::row,
              {1, 5, 9, 2, 6, 10, 3, 7, 11, 4, 8, 12, 13, 17, 21, 14, 18, 22, 15, 19, 23, 16, 20, 24});
        check(component::row, component::band, component::column,
              {1, 2, 3, 4, 13, 14, 15, 16, 5, 6, 7, 8, 17, 18, 19, 20, 9, 10, 11, 12, 21, 22, 23, 24});
        check(component::column, component::band, component::row,
              {1, 5, 9, 13, 17, 21, 2, 6, 10, 14, 18, 22, 3, 7, 11, 15, 19, 23, 4, 8, 12, 16, 20, 24});
        check(component::row, component::column, component::band,
              {1, 13, 2, 14, 3, 15, 4, 16, 5, 17, 6, 18, 7, 19, 8, 20, 9, 21, 10, 22, 11, 23, 12, 24});
        check(component::column, component::row, component::band,
              {1, 13, 5, 17, 9, 21, 2, 14, 6, 18, 10, 22, 3, 15, 7, 19, 11, 23, 4, 16, 8, 20, 12, 24});
    }

    TEST_F(xgdal_fixture, invalid_interleave)
    {
        load_gdal_options options;
        options.interleave = {component::band, component::column, component::column};
        EXPECT_THROW(load_gdal<int>(path, options), std::runtime_error);
    }

    TEST_F(xgdal_fixture, file_missing)
    {
        EXPECT_THROW(load_gdal<int>(path + "/file-dne.tif", {}), std::runtime_error);
    }

    TEST_F(xgdal_fixture, silent_downcast)
    {
        // Verify that GDAL silently downcasts the type from int to byte.
        auto test = load_gdal<uint8_t>(path, {});
        auto truth = xt::adapt(data, test.shape());
        EXPECT_TRUE(all(equal(xt::cast<uint8_t>(truth), test)));
    }

    TEST_F(xgdal_fixture, invalid_band_args)
    {
        load_gdal_options opt;
        opt.bands_to_load = {0}; // band out-of-range.
        EXPECT_THROW(load_gdal<int>(path, opt), std::runtime_error);

        opt.bands_to_load = {3}; // band out-of-range.
        EXPECT_THROW(load_gdal<int>(path, opt), std::runtime_error);
    }

    TEST_F(xgdal_fixture, selected_band)
    {
        load_gdal_options opt;
        opt.bands_to_load = {2};
        auto test = load_gdal<int>(path, opt);
        auto truth = xt::adapt(data.data() + nx * ny, test.shape());
        EXPECT_TRUE(all(equal(truth, test)));
    }

    TEST_F(xgdal_fixture, custom_error_handler)
    {
        bool thrown = false;
        load_gdal_options opt;
        opt.error_handler = [&](const std::string &)
        { thrown = true; };
        auto test = load_gdal<int>(path + "/file-dne.tif", opt);
        EXPECT_TRUE(thrown);
        EXPECT_EQ(test.size(), 0);
    }
}  // namespace xt
