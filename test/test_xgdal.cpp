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
            auto ds = GDALCreate(driver, in_path.c_str(), nx, ny, bands, GDT_Int32, nullptr);
            ASSERT_NE(ds, nullptr);
            auto err = GDALDatasetRasterIO(ds, GDALRWFlag::GF_Write, 0, 0, nx, ny, data.data(), nx, ny,
                                           GDALDataType::GDT_Int32, bands, nullptr, 0, 0, 0);
            ASSERT_EQ(err, CPLErr::CE_None);
            GDALClose(ds);
        }
        void TearDown() override
        {
            // Delete the in-memory datasets.
            VSIUnlink(in_path.c_str());
            VSIUnlink(out_path.c_str());
        }

        std::string in_path = "/vsimem/test_file.tif";
        std::string out_path = "/vsimem/test_file_output.tif";
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

    TEST_F(xgdal_fixture, load_invalid_interleave)
    {
        load_gdal_options options;
        options.interleave = {component::band, component::column, component::column};
        EXPECT_THROW(load_gdal<int>(in_path, options), std::runtime_error);
    }

    TEST_F(xgdal_fixture, load_file_missing)
    {
        EXPECT_THROW(load_gdal<int>(in_path + "/file-dne.tif", {}), std::runtime_error);
    }

    TEST_F(xgdal_fixture, load_silent_downcast)
    {
        // Verify that GDAL silently downcasts the type from int to byte.
        auto test = load_gdal<uint8_t>(in_path, {});
        auto truth = xt::adapt(data, test.shape());
        EXPECT_TRUE(all(equal(xt::cast<uint8_t>(truth), test)));
    }

    TEST_F(xgdal_fixture, load_invalid_band_args)
    {
        load_gdal_options opt;
        opt.bands_to_load = {0}; // band out-of-range.
        EXPECT_THROW(load_gdal<int>(in_path, opt), std::runtime_error);

        opt.bands_to_load = {3}; // band out-of-range.
        EXPECT_THROW(load_gdal<int>(in_path, opt), std::runtime_error);
    }

    TEST_F(xgdal_fixture, load_selected_band)
    {
        load_gdal_options opt;
        opt.bands_to_load = {2};
        auto test = load_gdal<int>(in_path, opt);
        auto truth = xt::adapt(data.data() + nx * ny, test.shape());
        EXPECT_TRUE(all(equal(truth, test)));
    }

    TEST_F(xgdal_fixture, load_custom_error_handler)
    {
        bool thrown = false;
        load_gdal_options opt;
        opt.error_handler = [&](const std::string &)
        { thrown = true; };
        auto test = load_gdal<int>(in_path + "/file-dne.tif", opt);
        EXPECT_TRUE(thrown);
        EXPECT_EQ(test.size(), 0);
    }

    TEST_F(xgdal_fixture, dump_invalid_interleave)
    {
        dump_gdal_options options;
        options.interleave = {component::band, component::column, component::column};
        EXPECT_THROW(dump_gdal(load_gdal<int>(in_path, {}), out_path, options), std::runtime_error);
    }

    TEST_F(xgdal_fixture, dump_gdal_missing_driver)
    {
        dump_gdal_options options;
        options.driver_name = "missing_driver";
        EXPECT_THROW(dump_gdal(load_gdal<int>(in_path, {}), out_path, options), std::runtime_error);
    }

    TEST_F(xgdal_fixture, dump_invalid_shape)
    {
        // Either too few or too many dimensions.
        dump_gdal_options options;
        xt::xtensor<int, 0> nil;
        EXPECT_THROW(dump_gdal(nil, out_path, {}), std::runtime_error);
        xt::xtensor<int, 4> d4;
        EXPECT_THROW(dump_gdal(d4, out_path, {}), std::runtime_error);
    }

    TEST_F(xgdal_fixture, dump_invalid_path)
    {
        dump_gdal_options options;
        EXPECT_THROW(dump_gdal(load_gdal<int>(in_path, {}), "/path/to/neverland", options), std::runtime_error);
    }

    TEST_F(xgdal_fixture, load_dump_interleave_roundtrip)
    {
        // Check the six interleave options passed through dump() and load().
        auto check = [&](component a, component b, component c, const std::vector<int> &vtruth)
        {
            load_gdal_options lopt;
            lopt.interleave = {a, b, c};
            auto test = load_gdal<int>(in_path, lopt);
            auto truth = xt::adapt(vtruth, test.shape());
            EXPECT_TRUE(all(equal(truth, test))) << "load failed";

            dump_gdal_options dopt;
            dopt.interleave = lopt.interleave;
            EXPECT_EQ(dump_gdal(truth, out_path, dopt), nullptr);
            test = load_gdal<int>(out_path, lopt);
            EXPECT_TRUE(all(equal(truth, test))) << "dump failed";
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

    TEST_F(xgdal_fixture, dump_supports_2d)
    {
        // Check that the 2D convenience hook is wired up.
        xtensor<int, 2> simple = {
            {1, 2, 3, 4},
            {5, 6, 7, 8}
        };
        dump_gdal(simple, out_path, {});
        auto test = load_gdal<int>(out_path, {});
        EXPECT_TRUE(all(equal(test, simple)));
    }

    TEST_F(xgdal_fixture, fancy_expression)
    {
        // Sanity check that fancy expressions are handled.
        xtensor<double, 2> values = {{ 2. }};
        dump_gdal(sqrt(values), out_path, {});
        auto test = load_gdal<double>(out_path, {});
        EXPECT_DOUBLE_EQ(test[0], std::sqrt(2.));
    }

    TEST_F(xgdal_fixture, dump_leaves_dataset_open)
    {
        dump_gdal_options opt;
        opt.return_opened_dataset = true;
        opt.creation_options.emplace_back("COMPRESS=DEFLATE");  // also try out an create_option.
        auto dataset = dump_gdal(load_gdal<int>(in_path, {}), out_path, opt);
        EXPECT_NE(dataset, nullptr);
        EXPECT_EQ(GDALGetRasterCount(dataset), bands);
        EXPECT_EQ(GDALGetRasterXSize(dataset), nx);
        EXPECT_EQ(GDALGetRasterYSize(dataset), ny);
        GDALClose(dataset);
    }
}  // namespace xt
