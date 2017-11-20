# ![xtensor-io](http://quantstack.net/assets/images/xtensor-io.svg)

[![Travis](https://travis-ci.org/QuantStack/xtensor-io.svg?branch=master)](https://travis-ci.org/QuantStack/xtensor-io)
[![ReadTheDocs](https://readthedocs.org/projects/xtensor-io/badge/?version=stable)](http://xtensor-io.readthedocs.io/en/stable/)
[![Join the Gitter Chat](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/QuantStack/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


xtensor-io offers a couple of functions to read and write images, audio files, and
NumPy's compressed storage format (NPZ) from C++ into xtensor data structures.
It makes use of well known libraries for image and audio handling (OpenImageIO and libsndfile).
Currently, only a few basic functions are provided, but upon demand we can consider to
improve the interface and offer more options to customize and improve the input-output
performance.

We haven't started building and testing on Windows, yet! Contributions welcome!

### Example

```cpp
// loads png image into xarray with shape WIDTH x HEIGHT x CHANNELS
auto arr = xt::load_image("test.png");

// write xarray out to JPEG image
xt::dump_image("dumptest.jpg", arr + 5);

// load npz file containing multiple arrays
auto npy_map = xt::load_npz("test.npz");

auto arr_0 = npy_map["arr_0"].cast<double>();
auto arr_1 = npy_map["arr_1"].cast<unsigned long>();

// open a wav file
auto audio = xt::load_audio("files/xtensor.wav");
auto& arr = std::get<1>(audio); // audio contents (like scipy.io.wavfile results)

// save a sine wave sound
int freq = 2000;
int sampling_freq = 44100;
double duration = 1.0;

auto t = xt::arange(0.0, duration, 1.0 / sampling_freq);
auto y = xt::sin(2.0 * numeric_constants<double>::PI * freq * t);

xt::dump_audio("files/sine.wav", y, sampling_freq);
```

### Installation

xtensor-io depends on thirdparty libraries for audio and image handling.

The easiest way to install xtensor-io (including all dependencies) is through conda:

```bash
conda install xtensor-io
```

To use xaudio, it's required to link with [`libsndfile`](http://www.mega-nerd.com/libsndfile/).
The library can be installed as follows:

```bash
# Conda
conda install libsndfile -c conda-forge
# Ubuntu / Debian
sudo apt-get install libsndfile-dev
# Fedora
sudo dnf install libsndfile-devel
```

For image handling, [`OpenImageIO`](http://openimageio.org/) is required.
We have a build of OpenImageIO on the conda QuantStack channel, but it can also
be optained from your favorite package manager on Linux. Note the conda build
is only tested with the `gcc-6` package from the QuantStack channel.

```bash
# Conda
(maybe: ) conda install gcc-6 -c QuantStack
conda install openimageio -c QuantStack
# Ubuntu / Debian
sudo apt-get install libopenimageio-dev
# Fedora
sudo dnf install OpenImageIO-devel
```

In order to decompress and read NPZ files, zlib is needed. It should be installed
on linux, but you can also obtain it:

```bash
# Conda
conda install zlib -c conda-forge
# Ubuntu / Debian
sudo apt-get install zlib1g-dev
# Fedora
sudo dnf install zlib-devel
```

From this directory:

```
mkdir build
cd build
cmake ..
make
sudo make install
```
