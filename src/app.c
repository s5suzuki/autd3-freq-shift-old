/*
 * File: app.c
 * Project: app_src
 * Created Date: 29/06/2020
 * Author: Shun Suzuki
 * -----
 * Last Modified: 26/04/2021
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2020 Hapis Lab. All rights reserved.
 *
 */

#include "app.h"

#include "iodefine.h"

#define CPU_VERSION (0xF000)  // v0.1-alpha-freq-shift

#define PADDING_SIZE (125)

#define BRAM_TR_SELECT (0x00)
#define BRAM_CONFIG_SELECT (0x0F)

#define CTRL_FLAGS_ADDR (0x00)
#define FPGA_INFO_ADDR (0x01)
#define CYCLE_CNT (0x10)
#define FPGA_VER_ADDR (0xFF)

#define CMD_RD_CPU_V_LSB (0x02)
#define CMD_RD_CPU_V_MSB (0x03)
#define CMD_RD_FPGA_V_LSB (0x04)
#define CMD_RD_FPGA_V_MSB (0x05)
#define CMD_CLEAR (0x09)
#define CMD_ULTRASOUND_CYCLE_CNT (0x0B)
#define CMD_WRITE_DUTY (0x10)
#define CMD_WRITE_PHASE (0x11)

extern RX_STR0 _sRx0;
extern RX_STR1 _sRx1;
extern TX_STR _sTx;

static volatile uint8_t _header_id = 0;

// fire when ethercat packet arrives
extern void recv_ethercat(void);
// fire once after power on
extern void init_app(void);
// fire periodically with 1ms interval
extern void update(void);

typedef enum {
  //
  //
  //
  //
  FORCE_FAN = 1 << 4,
} RxGlobalControlFlags;

typedef struct {
  uint8_t msg_id;
  uint8_t control_flags;
  uint8_t cmd;
  uint8_t _pad[PADDING_SIZE];
} RxGlobalHeader;

static void write_duty(volatile uint16_t *src, uint32_t size) {
  volatile uint16_t *base = (volatile uint16_t *)FPGA_BASE;
  uint32_t i;
  uint16_t base_addr;

  base_addr = get_addr(BRAM_TR_SELECT, 0);
  for (i = 0; i < size; i++) {
    base[base_addr + (i << 1) + 1] = src[i];
  }
}

static void write_phase(volatile uint16_t *src, uint32_t size) {
  volatile uint16_t *base = (volatile uint16_t *)FPGA_BASE;
  uint32_t i;
  uint16_t base_addr;

  base_addr = get_addr(BRAM_TR_SELECT, 0);
  for (i = 0; i < size; i++) {
    base[base_addr + (i << 1)] = src[i];
  }
}

static void clear(void) {
  volatile uint16_t *base = (volatile uint16_t *)FPGA_BASE;
  uint16_t addr;

  addr = get_addr(BRAM_TR_SELECT, 0);
  word_set_volatile(&base[addr], 0x0000, TRANS_NUM << 1);

  bram_write(BRAM_CONFIG_SELECT, CTRL_FLAGS_ADDR, 0);
  bram_write(BRAM_CONFIG_SELECT, CYCLE_CNT, 0);
}

void init_app(void) { clear(); }

static uint16_t get_cpu_version(void) { return CPU_VERSION; }

static uint16_t get_fpga_version(void) { return bram_read(BRAM_CONFIG_SELECT, FPGA_VER_ADDR); }

void update(void) {}

void recv_ethercat(void) {
  RxGlobalHeader *header = (RxGlobalHeader *)(_sRx1.data);

  if (header->msg_id != _header_id) {
    _header_id = header->msg_id;

    switch (header->cmd) {
      case CMD_WRITE_DUTY:
        bram_write(BRAM_CONFIG_SELECT, CTRL_FLAGS_ADDR, header->control_flags);
        write_duty(_sRx0.data, TRANS_NUM);
        _sTx.ack = ((uint16_t)(header->msg_id)) << 8;
        break;

      case CMD_WRITE_PHASE:
        bram_write(BRAM_CONFIG_SELECT, CTRL_FLAGS_ADDR, header->control_flags);
        write_phase(_sRx0.data, TRANS_NUM);
        _sTx.ack = ((uint16_t)(header->msg_id)) << 8;
        break;

      case CMD_ULTRASOUND_CYCLE_CNT:
        bram_write(BRAM_CONFIG_SELECT, CYCLE_CNT, _sRx0.data[0]);
        _sTx.ack = ((uint16_t)(header->msg_id)) << 8;
        break;

      case CMD_RD_CPU_V_LSB:
        _sTx.ack = ((((uint16_t)(header->msg_id)) << 8) & 0xFF00) | (get_cpu_version() & 0x00FF);
        break;

      case CMD_RD_CPU_V_MSB:
        _sTx.ack = ((((uint16_t)(header->msg_id)) << 8) & 0xFF00) | ((get_cpu_version() >> 8) & 0x00FF);
        break;

      case CMD_RD_FPGA_V_LSB:
        _sTx.ack = ((((uint16_t)(header->msg_id)) << 8) & 0xFF00) | (get_fpga_version() & 0x00FF);
        break;

      case CMD_RD_FPGA_V_MSB:
        _sTx.ack = ((((uint16_t)(header->msg_id)) << 8) & 0xFF00) | ((get_fpga_version() >> 8) & 0x00FF);
        break;

      case CMD_CLEAR:
        clear();
        break;

      default:
        break;
    }
  }
}
