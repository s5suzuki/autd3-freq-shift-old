# README

This firmware is for changing the ultrasound frequency.

Version: 0.1.0-alpha

# :fire: CAUTION

Some codes has omitted because they contain proprietary parts.

## Address map to FPGA

* BRAM_SELECT: High 4bit
* BRAM_ADDR: Low 12bit

| BRAM_SELECT | BRAM_ADDR | DATA                             | R/W |
|-------------|-----------|----------------------------------|-----|
| 0x0         | 0x000    | phase[0]                 | W   |
|             | 0x001    | duty[0]                 | W   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x1F0    | phase[248]                 | W   |
| 　          | 0x1F1    | duty[248]                 | W   |
| 　          | 0x1F2    | unused                           | -   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x1FF    | unused                           | -   |
| 　          | 0x200    | -                                | -   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0xFFF    | -                                 | -   | 　
| 0x2         | 0x000   | 7:0 = ctrl_flag                    | W   |
| 　          | 0x001   | 7:0 = fpga_info            | W   |
| 　          | 0x002   | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x00E   | unused                           | -　  |
| 　          | 0x010   | ultrasound cycle                           | W  |
| 　          | 0x011   | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x0FE   | unused                           | -　  |
| 　          | 0x0FF   | fpga_version                    | R   |
| 　          | 0x100   | -                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0xFFF   | -                           | -　  |

## Firmware version number

| Version number | Version |
|----------------|---------|
| 61440          | v0.1-alpha    |

# Author

Shun Suzuki, 2021-
