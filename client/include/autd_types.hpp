// File: autd_types.hpp
// Project: include
// Created Date: 26/12/2020
// Author: Shun Suzuki
// -----
// Last Modified: 14/04/2021
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

enum RX_GLOBAL_CONTROL_FLAGS {
  LOOP_BEGIN = 1 << 0,
  LOOP_END = 1 << 1,
  MOD_BEGIN = 1 << 2,
  SILENT = 1 << 3,
  FORCE_FAN = 1 << 4,
  SEQ_MODE = 1 << 5,
  SEQ_BEGIN = 1 << 6,
  SEQ_END = 1 << 7
};

constexpr size_t MOD_FRAME_SIZE = 124;

struct RxGlobalHeader {
  uint8_t msg_id;
  uint8_t control_flags;
  uint8_t cmd;
  uint8_t mod_size;
  uint8_t mod[MOD_FRAME_SIZE];
};
}  // namespace autd
