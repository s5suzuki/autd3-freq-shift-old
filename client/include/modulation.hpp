// File: modulation.hpp
// Project: include
// Created Date: 04/11/2018
// Author: Shun Suzuki
// -----
// Last Modified: 14/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2018-2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "autd_types.hpp"
#include "consts.hpp"
#include "result.hpp"

namespace autd {
namespace modulation {
class Modulation;
}
using ModulationPtr = std::shared_ptr<modulation::Modulation>;
}  // namespace autd

namespace autd::modulation {

/**
 * @brief Modulation controls the amplitude modulation
 */
class Modulation {
 public:
  Modulation() noexcept : _sent(0) {}
  virtual ~Modulation() = default;
  Modulation(const Modulation& v) noexcept = default;
  Modulation& operator=(const Modulation& obj) = default;
  Modulation(Modulation&& obj) = default;
  Modulation& operator=(Modulation&& obj) = default;

  /**
   * @brief Generate empty modulation, which produce static pressure
   */
  static ModulationPtr Create(uint8_t amp = 0xff) {
    auto mod = std::make_shared<Modulation>();
    mod->_buffer.resize(1, amp);
    return mod;
  }
  [[nodiscard]] virtual Result<bool, std::string> Build() { return Ok(true); }
  std::vector<uint8_t>& buffer() { return this->_buffer; }
  size_t& sent() { return this->_sent; }

 private:
  std::vector<uint8_t> _buffer;
  size_t _sent;
};
}  // namespace autd::modulation
