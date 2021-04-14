// File: ethercat_link.hpp
// Project: lib
// Created Date: 01/06/2016
// Author: Seki Inoue
// -----
// Last Modified: 14/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2016-2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

#include "link.hpp"

namespace autd::link {

/**
 * @brief TwinCATLink using local TwinCAT server
 */
class LocalTwinCATLink : public Link {
 public:
  /**
   * @brief Create LocalTwinCATLink.
   */
  static LinkPtr Create();
  LocalTwinCATLink() = default;
  ~LocalTwinCATLink() override = default;
  LocalTwinCATLink(const LocalTwinCATLink& v) noexcept = delete;
  LocalTwinCATLink& operator=(const LocalTwinCATLink& obj) = delete;
  LocalTwinCATLink(LocalTwinCATLink&& obj) = delete;
  LocalTwinCATLink& operator=(LocalTwinCATLink&& obj) = delete;

  Result<bool, std::string> Open() override = 0;
  Result<bool, std::string> Close() override = 0;
  Result<bool, std::string> Send(size_t size, const uint8_t* buf) override = 0;
  Result<bool, std::string> Read(uint8_t* rx, uint32_t buffer_len) override = 0;
  bool is_open() override = 0;
};
}  // namespace autd::link
