# xtensor-io

This library delivers a couple of function to efficiently in- and output
images and NumPy compressed arrays (npz) to xtensor data structures.
In order to read and write images, the ![OpenImageIO](https://github.com/OpenImageIO/oiio)
library is utilized. We only provide a few simple functions, but upon demand
can improve this interface.

```cpp
// loads png image into xarray with shape WIDTH x HEIGHT x CHANNELS
auto arr = xt::load_image("test.png");

// write xarray out to JPEG image
xt::dump_image("dumptest.jpg", arr + 5);

// load npz file containing multiple arrays
auto npy_map = xt::load_npz("test.npz");

auto arr_0 = npy_map["arr_0"].cast<double>();
auto arr_1 = npy_map["arr_1"].cast<unsigned long>();
```

## Installation

In order to install this library on linux systems, please find a copy
of `libopenimageio` (on fedora: `dnf install OpenImageIO-devel`,
ubuntu: `apt-get install libopenimageio-dev`). This should enable you 
to build the library like a regular cmake package:

From this directory:

```
mkdir build
cd build
cmake ..
make
sudo make install
```
