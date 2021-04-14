/*
 * File: app.c
 * Project: app_src
 * Created Date: 29/06/2020
 * Author: Shun Suzuki
 * -----
 * Last Modified: 14/04/2021
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2020 Hapis Lab. All rights reserved.
 *
 */

#include "app.h"

#include "iodefine.h"
#include "utils.h"

#define CPU_VERSION (0x1000)  // v1.0-lite

#define MICRO_SECONDS (1000)

#define MOD_BUF_SIZE (4000)
#define MOD_FRAME_SIZE (124)

#define BRAM_TR_SELECT (0)
#define BRAM_MOD_SELECT (1)
#define BRAM_CONFIG_SELECT (2)

#define CTRL_FLAGS_ADDR (0)
#define FPGA_INFO_ADDR (1)
#define CLK_SYNC_ADDR (2)
#define FPGA_VER_ADDR (255)

#define CMD_OP (0x00)
#define CMD_RD_CPU_V_LSB (0x02)
#define CMD_RD_CPU_V_MSB (0x03)
#define CMD_RD_FPGA_V_LSB (0x04)
#define CMD_RD_FPGA_V_MSB (0x05)
#define CMD_CLEAR (0x09)

extern RX_STR0 _sRx0;
extern RX_STR1 _sRx1;
extern TX_STR _sTx;

static volatile uint8_t _header_id = 0;
static volatile uint8_t _ctrl_flag = 0;

static volatile uint8_t _mod_buf[MOD_BUF_SIZE];
static volatile uint16_t _mod_size = 0;

// fire when ethercat packet arrives
extern void recv_ethercat(void);
// fire once after power on
extern void init_app(void);
// fire periodically with 1ms interval
extern void update(void);

typedef enum {
  MOD_BEGIN = 1 << 0,
  MOD_END = 1 << 1,
  MOD_SYNC = 1 << 2,
  SILENT = 1 << 3,
  FORCE_FAN = 1 << 4,
} RxGlobalControlFlags;

typedef struct {
  uint8_t msg_id;
  uint8_t control_flags;
  uint8_t cmd;
  uint8_t mod_size;
  uint8_t mod[MOD_FRAME_SIZE];
} RxGlobalHeader;

static void write_mod_buf() {
  volatile uint16_t *base = (volatile uint16_t *)FPGA_BASE;
  uint16_t addr = get_addr(BRAM_MOD_SELECT, 0);
  word_cpy_volatile(&base[addr], (volatile uint16_t *)_mod_buf, MOD_BUF_SIZE >> 1);
}

static void clear(void) {
  volatile uint16_t *base = (volatile uint16_t *)FPGA_BASE;
  uint16_t addr;

  memset_volatile(_mod_buf, 0xff, MOD_BUF_SIZE);
  write_mod_buf(MOD_BUF_SIZE);

  addr = get_addr(BRAM_TR_SELECT, 0);
  word_set_volatile(&base[addr], 0x0000, TRANS_NUM);

  bram_write(BRAM_CONFIG_SELECT, CTRL_FLAGS_ADDR, SILENT);
}

void init_app(void) { clear(); }

static void sync_fpga_mod_clk(void) {
  volatile uint32_t sys_time;
  volatile uint64_t next_sync0 = ECATC.DC_CYC_START_TIME.LONGLONG;

  // wait for sync0 activation
  while (ECATC.DC_SYS_TIME.LONGLONG < next_sync0) {
    wait_ns(1000 * MICRO_SECONDS);
  }

  sys_time = mod1e9_u64(ECATC.DC_SYS_TIME.LONGLONG + 1000 * MICRO_SECONDS);
  while (sys_time > 50 * MICRO_SECONDS) {
    sys_time = mod1e9_u64(ECATC.DC_SYS_TIME.LONGLONG + 1000 * MICRO_SECONDS);
  }
  wait_ns(50 * MICRO_SECONDS);
  bram_write(BRAM_CONFIG_SELECT, CLK_SYNC_ADDR, 1);
  asm volatile("dmb");
}

static uint16_t get_cpu_version(void) { return CPU_VERSION; }

static uint16_t get_fpga_version(void) { return bram_read(BRAM_CONFIG_SELECT, FPGA_VER_ADDR); }

void update(void) {
  if ((_ctrl_flag & MOD_SYNC) != 0) {
    sync_fpga_mod_clk();
    _ctrl_flag &= ~MOD_SYNC;
  }
}

void recv_ethercat(void) {
  volatile uint16_t *base = (volatile uint16_t *)FPGA_BASE;
  RxGlobalHeader *header = (RxGlobalHeader *)(_sRx1.data);
  uint16_t mod_write;
  uint16_t addr;
  uint32_t i;

  if (header->msg_id != _header_id) {
    _ctrl_flag = header->control_flags;
    _header_id = header->msg_id;

    switch (header->cmd) {
      case CMD_OP:
        bram_write(BRAM_CONFIG_SELECT, CTRL_FLAGS_ADDR, _ctrl_flag);
        addr = get_addr(BRAM_TR_SELECT, 0);
        word_cpy_volatile(&base[addr], _sRx0.data, TRANS_NUM);

        if ((header->control_flags & MOD_BEGIN) != 0) {
          _mod_size = 0;
        }

        memcpy_volatile(&_mod_buf[_mod_size], header->mod, header->mod_size);
        _mod_size += header->mod_size;

        if ((header->control_flags & MOD_END) != 0) {
          for (i = _mod_size; i < MOD_BUF_SIZE; i += _mod_size) {
            mod_write = (i + _mod_size) > MOD_BUF_SIZE ? MOD_BUF_SIZE - i : _mod_size;
            memcpy_volatile(&_mod_buf[i], &_mod_buf[0], mod_write);
          }
          write_mod_buf();
        }
        _sTx.ack = ((uint16_t)(header->msg_id)) << 8;
        break;

      case CMD_RD_CPU_V_LSB:
        _sTx.ack = (((uint16_t)(header->msg_id)) << 8) | (get_cpu_version() & 0x00FF);
        break;

      case CMD_RD_CPU_V_MSB:
        _sTx.ack = (((uint16_t)(header->msg_id)) << 8) | ((get_cpu_version() & 0xFF00) >> 8);
        break;

      case CMD_RD_FPGA_V_LSB:
        _sTx.ack = (((uint16_t)(header->msg_id)) << 8) | (get_fpga_version() & 0x00FF);
        break;

      case CMD_RD_FPGA_V_MSB:
        _sTx.ack = (((uint16_t)(header->msg_id)) << 8) | ((get_fpga_version() & 0xFF00) >> 8);
        break;

      case CMD_CLEAR:
        clear();
        break;

      default:
        break;
    }
  }
}
