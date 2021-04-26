# README

This firmware is for changing the ultrasound frequency.

Version: 0.1.0-alpha

The code is written in SystemVerilog with Vivado 2020.2.

# Connection

* [IN] [16:0] CPU_ADDR,
* [IN/OUT] [15:0] CPU_DATA
* [OUT] [252:1] XDCR_OUT
* [IN] CPU_CKIO
* [IN] CPU_CS1_N
* [IN] RESET_N
* [IN] CPU_WE0_N
* [IN] CPU_WE1_N
* [IN] CPU_RD_N
* [IN] CPU_RDWR
* [IN] MRCC_25P6M
* [IN] CAT_SYNC0
* [OUT] FORCE_FAN
* [IN] THERMO
* [IN] [3:0] GPIO_IN
* [OUT] [3:0] GPIO_OUT

# Address map

* BRAM_SELECT: High 4bit
* BRAM_ADDR: Low 12bit

| BRAM_SELECT | BRAM_ADDR | DATA 32 bit                           | R/W |
|-------------|-----------|----------------------------------|-----|
| 0x0         | 0x000    | duty[0]/phase[0]                 | R   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x0F8    | duty[248]/phase[248]                 | R   |
| 　          | 0x0F9    | unused                           | -   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x0FF    | unused                           | -   |

| BRAM_SELECT | BRAM_ADDR | DATA 16 bit                          | R/W |
|-------------|-----------|----------------------------------|-----|
| 0x2         | 0x000   | 7:0 = ctrl_flag                    | R   |
| 　          | 0x001   | 7:0 = fpga_info            | W   |
| 　          | 0x002   | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x00E   | unused                           | -　  |
| 　          | 0x010   | ultrasound cycle                           | R  |
| 　          | 0x011   | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x0FE   | unused                           | -　  |
| 　          | 0x0FF   | fpga_version                    | -   |

## Firmware version number

| Version number | Version |
|----------------|---------|
| 61440          | v0.1-alpha    |

# Author

Shun Suzuki, 2021-
