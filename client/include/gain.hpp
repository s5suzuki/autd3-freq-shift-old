// File: gain.hpp
// Project: include
// Created Date: 11/04/2018
// Author: Shun Suzuki
// -----
// Last Modified: 16/04/2021
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

  static void CheckAndInit(const std::vector<Device>& devices, std::vector<AUTDDataArray>* data) {
    assert(devices.size() != 0);

    data->clear();

    const auto num_device = devices.size();
    data->resize(num_device);
  }

  /**
   * @brief Generate empty gain
   */
  static GainPtr Create() { return std::make_shared<Gain>(); }

  /**
   * @brief Calculate amplitude and phase of each transducer
   */
  [[nodiscard]] virtual Result<bool, std::string> Build(std::vector<Device>& devices) {
    if (this->built()) return Ok(false);

    CheckAndInit(devices, &this->_data);

    for (size_t i = 0; i < devices.size(); i++) this->_data[i].fill(0x0000);

    this->_built = true;
    return Ok(true);
  }

  /**
   * @brief Re-calculate amplitude and phase of each transducer
   */
  [[nodiscard]] virtual Result<bool, std::string> Rebuild(std::vector<Device>& devices) {
    this->_built = false;
    return this->Build(devices);
  }

  /**
   * @brief Getter function for the data of amplitude and phase of each transducers
   * @details Each data is 16 bit unsigned integer, where MSB represents amplitude and LSB represents phase
   */
  std::vector<AUTDDataArray>& data() { return this->_data; }

  Gain() noexcept : _built(false) {}
  virtual ~Gain() = default;
  Gain(const Gain& v) noexcept = default;
  Gain& operator=(const Gain& obj) = default;
  Gain(Gain&& obj) = default;
  Gain& operator=(Gain&& obj) = default;

 protected:
  bool _built;
  std::vector<AUTDDataArray> _data;

  [[nodiscard]] bool built() const noexcept { return this->_built; }
};

using NullGain = Gain;

}  // namespace autd
