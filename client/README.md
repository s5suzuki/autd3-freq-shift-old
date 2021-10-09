# README

## Windows

run `build.ps1` or run CMake, then open `autd3-freq-shift.sln` in `BUILD_DIR` (default `./build`)

### build.ps1 options

[] is a default.

* -BUILD_DIR = [./build]
* -VS_VERSION = 2017, [2019], 2022
* -ARCH = [x64]

## Mac/Linux

```
mkdir build && cd build
cmake ..
make
sudo examples/example_soem
```

# Author

Shun Suzuki, 2021
