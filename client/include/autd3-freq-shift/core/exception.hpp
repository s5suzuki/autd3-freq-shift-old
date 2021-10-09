// File: exception.hpp
// Project: core
// Created Date: 04/07/2021
// Author: Shun Suzuki
// -----
// Last Modified: 09/10/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <stdexcept>
#include <string>

namespace autd::core::exception {

class GainBuildError : public std::runtime_error {
 public:
  explicit GainBuildError(std::string const& message) : std::runtime_error(message) {}
  explicit GainBuildError(char const* message) : std::runtime_error(message) {}
};

class TimerError : public std::runtime_error {
 public:
  explicit TimerError(std::string const& message) : std::runtime_error(message) {}
  explicit TimerError(char const* message) : std::runtime_error(message) {}
};

class LinkError : public std::runtime_error {
 public:
  explicit LinkError(std::string const& message) : std::runtime_error(message) {}
  explicit LinkError(char const* message) : std::runtime_error(message) {}
};

}  // namespace autd::core::exception
