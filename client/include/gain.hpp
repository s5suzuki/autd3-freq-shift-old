// File: gain.hpp
// Project: include
// Created Date: 11/04/2018
// Author: Shun Suzuki
// -----
// Last Modified: 26/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2018-2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "autd_types.hpp"
#include "consts.hpp"
#include "linalg.hpp"
#include "result.hpp"

namespace autd {
class Gain;
using GainPtr = std::shared_ptr<Gain>;

/**
 * @brief Gain controls the amplitude and phase of each transducer in the AUTD
 */
class Gain {
 public:
  static uint8_t ToDuty(const Float amp) noexcept {
    const auto d = std::asin(amp) / PI;  //  duty (0 ~ 0.5)
    return static_cast<uint8_t>(511 * d);
  }

  static void CheckAndInit(const std::vector<Device>& devices, std::vector<AUTDDataArray>* duty, std::vector<AUTDDataArray>* phase) {
    assert(devices.size() != 0);

    duty->clear();
    phase->clear();

    const auto num_device = devices.size();
    duty->resize(num_device);
    phase->resize(num_device);
  }

  /**
   * @brief Generate empty gain
   */
  static GainPtr Create() { return std::make_shared<Gain>(); }

  /**
   * @brief Calculate amplitude and phase of each transducer
   */
  [[nodiscard]] virtual Result<bool, std::string> Build(std::vector<Device>& devices, std::vector<uint16_t>& freq_cycle) {
    if (this->built()) return Ok(false);

    CheckAndInit(devices, &this->_duties, &this->_phases);

    for (size_t i = 0; i < devices.size(); i++) this->_duties[i].fill(0x0000);
    for (size_t i = 0; i < devices.size(); i++) this->_phases[i].fill(0x0000);

    this->_built = true;
    return Ok(true);
  }

  /**
   * @brief Re-calculate amplitude and phase of each transducer
   */
  [[nodiscard]] virtual Result<bool, std::string> Rebuild(std::vector<Device>& devices, std::vector<uint16_t>& freq_cycle) {
    this->_built = false;
    return this->Build(devices, freq_cycle);
  }

  /**
   * @brief Getter function for the duty of amplitude and phase of each transducers
   * @details Each duty is 16 bit unsigned integer
   */
  std::vector<AUTDDataArray>& duty() { return this->_duties; }

  /**
   * @brief Getter function for the phase of amplitude and phase of each transducers
   * @details Each phase is 16 bit unsigned integer
   */
  std::vector<AUTDDataArray>& phase() { return this->_phases; }

  Gain() noexcept : _built(false) {}
  virtual ~Gain() = default;
  Gain(const Gain& v) noexcept = default;
  Gain& operator=(const Gain& obj) = default;
  Gain(Gain&& obj) = default;
  Gain& operator=(Gain&& obj) = default;

 protected:
  bool _built;
  std::vector<AUTDDataArray> _duties;
  std::vector<AUTDDataArray> _phases;

  [[nodiscard]] bool built() const noexcept { return this->_built; }
};

using NullGain = Gain;

}  // namespace autd
