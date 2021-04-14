![build](https://github.com/shinolab/autd3-library-software/workflows/build/badge.svg)
![Upload Release Asset](https://github.com/shinolab/autd3-library-software/workflows/Upload%20Release%20Asset/badge.svg)

# [AUTD3-Lite](https://hapislab.org/airborne-ultrasound-tactile-display?lang=en)

Lightweight version of AUTD3 library.

Version: 1.0.0-lite

* The firmware codes are available at [here](https://github.com/shinolab/autd3-lite-firmware).

## :memo: Versioning

The meanings of version number x.y.z are
* x: Firmware version
* y: Software version
* z: patch version

If the number of x changes, the firmware of FPGA or CPU must be upgraded.

## :fire: CAUTION

* Before using, be sure to write the latest firmwares in `dist/firmware`. For more information, please see [readme](/dist/firmware/Readme.md).

## :ballot_box_with_check: Requirements

* If you use `SOEMLink` on Windows, install [Npcap](https://nmap.org/npcap/) with WinPcap API-compatible mode (recommended) or [WinPcap](https://www.winpcap.org/).

* If you use `TwinCAT`, please see [how to install AUTDServer](https://github.com/shinolab/autd3-library-software/wiki/How-to-install-AUTDServer).

## :beginner: Example

See `client/examples`

If you are using Linux/macOS, you may need to run as root.

## :copyright: LICENSE

See [LICENSE](./LICENSE) and [NOTICE](./NOTICE).

# Author

Shun Suzuki, 2021
