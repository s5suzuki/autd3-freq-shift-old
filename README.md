# README

This is a lightweight CPU firmware of AUTD3.

Version: 1.0-lite

# :fire: CAUTION

Some codes has omitted because they contain proprietary parts.

## Address map to FPGA

* BRAM_SELECT: High 4bit
* BRAM_ADDR: Low 12bit

| BRAM_SELECT | BRAM_ADDR | DATA                             | R/W |
|-------------|-----------|----------------------------------|-----|
| 0x0         | 0x000    | duty[0]/phase[0]                 | W   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x0F8    | duty[248]/phase[248]                 | W   |
| 　          | 0x0F9    | unused                           | -   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x0FF    | unused                           | -   |
| 　          | 0x100    | -                                | -   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x1FF    | -                                 | -   | 　
| 0x1         | 0x000    | mod[1]/mod[0]                    | W   |
| 　          | 0x001    | mod[3]/mod[2]                    | W   |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x7CF    | mod[3999]/mod[3998]              | W   |
| 　          | 0x7D0    | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x7FF    | unused                           | -　  |
| 　          | 0x800    | -                              | -　  |
| 　          | ︙        | ︙                               | ︙　  |
| 　          | 0xFFF    | -                              | 　-  |
| 0x2         | 0x000   | 7:0 = ctrl_flag                    | W   |
| 　          | 0x001   | 7:0 = fpga_info            | W   |
| 　          | 0x002   | 0 = mod_clk_sync_flag               | W   |
| 　          | 0x003   | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x0FE   | unused                           | -　  |
| 　          | 0x0FF   | fpga_version                    | R   |
| 　          | 0x100   | -                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0xFFF   | -                           | -　  |

## Firmware version number

| Version number | Version |
|----------------|---------|
| 4096           | v1.0-lite    |

# Author

Shun Suzuki, 2021-
