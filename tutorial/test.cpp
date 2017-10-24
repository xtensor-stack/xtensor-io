#include "xtensor-io/ximageio.hpp"
#include "xtensor-io/xnpz.hpp"

#include "xtensor/xarray.hpp"
#include "xtensor/xmath.hpp"

#include "xtensor/xio.hpp"
#include "xtensor/xinfo.hpp"

int main()
{
    // auto arr = xt::load_image("test.jpg");

    // std::cout << arr << std::endl;
    // std::cout << xt::info(arr) << std::endl;

    // unsigned char minus = 50;
    // arr -= minus;
    // std::cout << xt::info(arr) << std::endl;
    // unsigned char max = 255, min = 0;
    // arr = xt::clip(arr, min, max);

    // dump_image("threshold.jpg", arr);

    xt::xarray<double> arr = {{1,2,3}, {4,5,6}};
    xt::dump_npz("abc.npz", "hackka123", arr, true);
    xt::dump_npz("abc.npz", "hackka", arr, true);
    // unsigned char min = 3, max = 5;
    // arr = xt::clip(arr, min, max);


    auto hackka_loaded = xt::load_npz<double>("abc.npz", "hackka");
    std::cout << hackka_loaded << std::endl;

    // auto npy_file = xt::load_npy_file(zstream, false);
    // std::cout << npy_file.cast<double>();
    // auto npz_map = xt::load_npz("npztestcompressed.npz");
    // auto arr_out = npz_map["hackka"].cast<double>();
    // std::cout << arr_out << std::endl;
    // auto arr1 = npz_map["arr_1"].cast<long>();
    // std::cout << arr1 << std::endl;

    return 0;
}