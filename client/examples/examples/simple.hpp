// File: simple.hpp
// Project: examples
// Created Date: 19/05/2020
// Author: Shun Suzuki
// -----
// Last Modified: 26/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <utility>

#include "autd3.hpp"

using autd::NUM_TRANS_X, autd::NUM_TRANS_Y, autd::TRANS_SIZE_MM, autd::NUM_TRANS_IN_UNIT;
using autd::Vector3;

constexpr autd::Float ULTRASOUND_WAVELENGTH = 8.5f;

class FocalPointGain final : public autd::Gain {
 public:
  static autd::GainPtr Create(const Vector3& point) { return std::make_shared<FocalPointGain>(std::forward<const Vector3>(point)); }

  autd::Result<bool, std::string> Build(std::vector<autd::Device>& devices, std::vector<uint16_t>& freq_cycle) override {
    if (this->_built) return autd::Ok(false);

    CheckAndInit(devices, &this->_duties, &this->_phases);

    for (size_t dev_idx = 0; dev_idx < devices.size(); dev_idx++) {
      const uint16_t duty = freq_cycle[dev_idx] >> 1;  // 50%
      const auto& tr_positions = devices[dev_idx].global_trans_positions();
      for (size_t i = 0; i < NUM_TRANS_IN_UNIT; i++) {
        const auto& trp = tr_positions[i];
        const auto dist = (trp - this->_point).norm();
        const auto f_phase = fmod(dist, ULTRASOUND_WAVELENGTH) / ULTRASOUND_WAVELENGTH;
        const auto phase = static_cast<uint16_t>(std::round(static_cast<float>(freq_cycle[dev_idx]) * (1 - f_phase)));
        this->_duties[dev_idx][i] = duty;
        this->_phases[dev_idx][i] = phase;
      }
    }

    this->_built = true;
    return autd::Ok(true);
  }

  explicit FocalPointGain(const Vector3& point) : Gain(), _point(std::forward<const Vector3>(point)) {}

 private:
  Vector3 _point = Vector3::Zero();
};

inline void simple_test(autd::ControllerPtr& cnt) {
  const auto center = Vector3(TRANS_SIZE_MM * ((NUM_TRANS_X - 1) / 2.0f), TRANS_SIZE_MM * ((NUM_TRANS_Y - 1) / 2.0f), 150.0f);
  const auto g = FocalPointGain::Create(center);
  cnt->SendBlocking(g).unwrap();
}
