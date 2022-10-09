// File: hardware_defined.hpp
// Project: core
// Created Date: 14/04/2021
// Author: Shun Suzuki
// -----
// Last Modified: 09/10/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <array>
#include <cstdint>

namespace autd {
namespace core {
constexpr size_t NUM_TRANS_IN_UNIT = 249;
constexpr size_t NUM_TRANS_X = 18;
constexpr size_t NUM_TRANS_Y = 14;
constexpr double TRANS_SPACING_MM = 10.16;
constexpr double DEVICE_WIDTH = 192.0;
constexpr double DEVICE_HEIGHT = 151.4;

template <typename T>
constexpr auto is_missing_transducer(T x, T y) {
  return y == 1 && (x == 1 || x == 2 || x == 16);
}

constexpr size_t ULTRASOUND_FREQUENCY = 40000;
constexpr uint32_t FPGA_BASE_CLK_FREQ = 200000000;
constexpr uint32_t FPGA_BASE_CLK_PERIOD_NS = 1000000000 / FPGA_BASE_CLK_FREQ;

using DataArray = std::array<uint16_t, NUM_TRANS_IN_UNIT>;

enum class COMMAND : uint8_t {
  OP = 0x00,
  READ_CPU_VER_LSB = 0x02,
  READ_CPU_VER_MSB = 0x03,
  READ_FPGA_VER_LSB = 0x04,
  READ_FPGA_VER_MSB = 0x05,
  SEQ_FOCI_MODE = 0x06,
  CLEAR = 0x09,
  ULTRASOUND_CYCLE_CNT = 0x0B,
  WRITE_DUTY = 0x10,
  WRITE_PHASE = 0x11
};

constexpr size_t FRAME_PADDING_SIZE = 125;

/**
 * \brief Data header common to all devices
 */
struct GlobalHeader {
  uint8_t msg_id;
  uint8_t _control_flags;
  COMMAND command;
  uint8_t mod[FRAME_PADDING_SIZE];
};

}  // namespace core
}  // namespace autd
