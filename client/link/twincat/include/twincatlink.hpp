// File: twincat_link.hpp
// Project: include
// Created Date: 14/04/2021
// Author: Shun Suzuki
// -----
// Last Modified: 21/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <Windows.h>

#include <memory>
#include <string>

#include "link.hpp"

namespace autd::link {

struct AmsNetId {
  uint8_t b[6];
};

struct AmsAddr {
  AmsNetId net_id;
  uint16_t port;
};

/**
 * @brief TwinCATLink using local TwinCAT server
 */
class TwinCATLink final : public Link {
 public:
  /**
   * @brief Create TwinCATLink.
   */
  static LinkPtr Create();
  TwinCATLink() : _port(0), _net_id(), _lib(nullptr) {}
  ~TwinCATLink() override = default;
  TwinCATLink(const TwinCATLink& v) noexcept = delete;
  TwinCATLink& operator=(const TwinCATLink& obj) = delete;
  TwinCATLink(TwinCATLink&& obj) = delete;
  TwinCATLink& operator=(TwinCATLink&& obj) = delete;

  Result<bool, std::string> Open() override;
  Result<bool, std::string> Close() override;
  Result<bool, std::string> Send(size_t size, const uint8_t* buf) override;
  Result<bool, std::string> Read(uint8_t* rx, size_t buffer_len) override;
  bool is_open() override;

 private:
  long _port;  // NOLINT
  AmsNetId _net_id;
  HMODULE _lib;
};
}  // namespace autd::link
