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

void SOEMController::SetupSync0(const bool activate, const std::vector<uint16_t>& freq_cycles) const {
  for (size_t slave = 1; slave <= _dev_num; slave++) {
    const uint32_t cycle_time_ns = FPGA_BASE_CLK_PERIOD_NS * freq_cycles[slave - 1] * 40;
    ec_dcsync0(static_cast<uint16_t>(slave), activate, cycle_time_ns, 0);
  }
}
