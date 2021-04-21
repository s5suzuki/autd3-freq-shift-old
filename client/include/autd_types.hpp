// File: autd_types.hpp
// Project: include
// Created Date: 26/12/2020
// Author: Shun Suzuki
// -----
// Last Modified: 16/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#pragma once

namespace autd {

#ifdef USE_DOUBLE_AUTD
using Float = double;
#else
using Float = float;
#endif

enum class RX_GLOBAL_CONTROL_FLAGS : uint8_t {
  NONE = 0,
  MOD_BEGIN = 1 << 0,
  MOD_END = 1 << 1,
  //
  SILENT = 1 << 3,
  FORCE_FAN = 1 << 4,
};

constexpr RX_GLOBAL_CONTROL_FLAGS operator|(RX_GLOBAL_CONTROL_FLAGS l, RX_GLOBAL_CONTROL_FLAGS r) noexcept {
  return static_cast<RX_GLOBAL_CONTROL_FLAGS>(static_cast<std::underlying_type<RX_GLOBAL_CONTROL_FLAGS>::type>(l) |
                                              static_cast<std::underlying_type<RX_GLOBAL_CONTROL_FLAGS>::type>(r));
}

constexpr RX_GLOBAL_CONTROL_FLAGS& operator|=(RX_GLOBAL_CONTROL_FLAGS& l, RX_GLOBAL_CONTROL_FLAGS r) noexcept {
  l = l | r;
  return l;
}

constexpr size_t MOD_FRAME_SIZE = 124;

struct RxGlobalHeader {
  uint8_t msg_id;
  RX_GLOBAL_CONTROL_FLAGS control_flags;
  uint8_t cmd;
  uint8_t mod_size;
  uint8_t mod[MOD_FRAME_SIZE];
};
}  // namespace autd
