// File: utils.hpp
// Project: core
// Created Date: 08/06/2021
// Author: Shun Suzuki
// -----
// Last Modified: 09/10/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <algorithm>
#include <cmath>

#include "autd3-freq-shift/core/hardware_defined.hpp"

namespace autd::core {

class Utilities {
 public:
  /**
   * \brief Convert ultrasound amplitude to duty ratio.
   * \param amp ultrasound amplitude (0 to 1). The amp will be clamped in the range [0, 1].
   * \return duty ratio
   */
  inline static uint16_t to_duty(const double amp, const uint16_t freq_cycle) noexcept {
    const auto d = std::asin(std::clamp(amp, 0.0, 1.0)) / M_PI;  //  duty (0 ~ 0.5)
    return static_cast<uint16_t>(std::round(d * static_cast<double>(freq_cycle)));
  }

  /**
   * \brief Convert phase in radian to descrete phase used in device.
   * \param phase phase in radian
   * \return descrete phase
   */
  inline static uint16_t to_phase(double phase, const uint16_t freq_cycle) noexcept {
    phase = std::fmod(phase + M_PI, 2.0 * M_PI);
    phase = phase / (2.0 * M_PI) * static_cast<double>(freq_cycle);
    return static_cast<uint16_t>(std::round(phase));
  }

  /**
   * \brief Pack two uint8_t value to uint16_t
   * \param high high byte
   * \param low high byte
   * \return packed uint16_t value
   */
  inline static uint16_t pack_to_u16(const uint8_t high, const uint8_t low) noexcept {
    auto res = static_cast<uint16_t>(low) & 0x00FF;
    res |= (static_cast<uint16_t>(high) << 8) & 0xFF00;
    return res;
  }
};
}  // namespace autd::core
