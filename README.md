# README

This is a lightweight FPGA firmware of AUTD3.

Version: 1.0.0-lite

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

| BRAM_SELECT | BRAM_ADDR | DATA                             | R/W |
|-------------|-----------|----------------------------------|-----|
| 0x0         | 0x000    | duty[0]/phase[0]                 | R   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x0F8    | duty[248]/phase[248]                 | R   |
| 　          | 0x0F9    | unused                           | -   |
| 　          | ︙      | ︙                              | ︙   |
| 　          | 0x0FF    | unused                           | -   |
| 0x1         | 0x000    | mod[0]                    | R   |
| 　          | 0x001    | mod[1]                    | R  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0xF9F    | mod[3999]              | R   |
| 　          | 0xFA0    | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0xFFF    | unused                           | -　  |
| 0x2         | 0x000   | 7:0 = ctrl_flag                    | R   |
| 　          | 0x001   | 7:0 = fpga_info            | W   |
| 　          | 0x002   | 0 = mod_clk_sync_flag               | R   |
| 　          | 0x003   | unused                           | -  |
| 　          | ︙        | ︙                               | ︙  |
| 　          | 0x0FE   | unused                           | -　  |
| 　          | 0x0FF   | fpga_version                    | -   |

## Firmware version number

| Version number | Version |
|----------------|---------|
| 4096           | v1.0-lite    |

# Author

Shun Suzuki, 2021-
