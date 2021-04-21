// File: twincat_link.cpp
// Project: src
// Created Date: 14/04/2021
// Author: Shun Suzuki
// -----
// Last Modified: 21/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#include "twincatlink.hpp"

#include <winnt.h>

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace autd::link {

constexpr uint32_t INDEX_GROUP = 0x3040030;
constexpr uint32_t INDEX_OFFSET_BASE = 0x81000000;
constexpr uint32_t INDEX_OFFSET_BASE_READ = 0x80000000;
constexpr uint16_t PORT = 301;

typedef long(_stdcall* tc_ads_port_open_ex)();                        // NOLINT
typedef long(_stdcall* tc_ads_port_close_ex)(long);                   // NOLINT
typedef long(_stdcall* tc_ads_get_local_address_ex)(long, AmsAddr*);  // NOLINT
typedef long(_stdcall* tc_ads_sync_write_req_ex)(long, AmsAddr*,      // NOLINT
                                                 unsigned long,       // NOLINT
                                                 unsigned long,       // NOLINT
                                                 unsigned long,       // NOLINT
                                                 void*);              // NOLINT
typedef long(_stdcall* tc_ads_sync_read_req_ex)(long, AmsAddr*,       // NOLINT
                                                unsigned long,        // NOLINT
                                                unsigned long,        // NOLINT
                                                unsigned long,        // NOLINT
                                                void*,                // NOLINT
                                                unsigned long*);      // NOLINT

constexpr auto TCADS_ADS_PORT_OPEN_EX = "AdsPortOpenEx";
constexpr auto TCADS_ADS_GET_LOCAL_ADDRESS_EX = "AdsGetLocalAddressEx";
constexpr auto TCADS_ADS_PORT_CLOSE_EX = "AdsPortCloseEx";
constexpr auto TCADS_ADS_SYNC_WRITE_REQ_EX = "AdsSyncWriteReqEx";
constexpr auto TCADS_ADS_SYNC_READ_REQ_EX = "AdsSyncReadReqEx2";

LinkPtr TwinCATLink::Create() {
  LinkPtr link = std::make_unique<TwinCATLink>();
  return link;
}

bool TwinCATLink::is_open() { return this->_port > 0; }

Result<bool, std::string> TwinCATLink::Open() {
  this->_lib = LoadLibrary("TcAdsDll.dll");
  if (_lib == nullptr) return Err(std::string("couldn't find TcADS-DLL"));

  const auto port_open = reinterpret_cast<tc_ads_port_open_ex>(GetProcAddress(this->_lib, TCADS_ADS_PORT_OPEN_EX));
  this->_port = (*port_open)();
  if (!this->_port) return Err(std::string("Error: Failed to open a new ADS port"));

  AmsAddr addr{};
  const auto get_addr = reinterpret_cast<tc_ads_get_local_address_ex>(GetProcAddress(this->_lib, TCADS_ADS_GET_LOCAL_ADDRESS_EX));
  if (const auto ret = get_addr(this->_port, &addr); ret) {
    std::stringstream ss;
    ss << "Error: AdsGetLocalAddress: " << std::hex << ret;
    return Err(ss.str());
  }

  this->_net_id = addr.net_id;
  return Ok(true);
}

Result<bool, std::string> TwinCATLink::Close() {
  const auto port_close = reinterpret_cast<tc_ads_port_close_ex>(GetProcAddress(this->_lib, TCADS_ADS_PORT_CLOSE_EX));
  const auto res = (*port_close)(this->_port);
  this->_port = 0;
  if (res == 0) return Ok(true);
  std::stringstream ss;
  ss << "Error on closing (local): " << std::hex << res;
  return Err(ss.str());
}

Result<bool, std::string> TwinCATLink::Send(const size_t size, const uint8_t* buf) {
  AmsAddr addr = {this->_net_id, PORT};
  const auto write = reinterpret_cast<tc_ads_sync_write_req_ex>(GetProcAddress(this->_lib, TCADS_ADS_SYNC_WRITE_REQ_EX));
  const auto ret = write(this->_port,  // NOLINT
                         &addr, INDEX_GROUP, INDEX_OFFSET_BASE,
                         static_cast<unsigned long>(size),  // NOLINT
                         const_cast<void*>(static_cast<const void*>(buf)));

  if (ret == 0) return Ok(true);
  // 6 : target port not found
  std::stringstream ss;
  ss << "Error on sending data (local): " << std::hex << ret;
  return Err(ss.str());
}

Result<bool, std::string> TwinCATLink::Read(uint8_t* rx, const size_t buffer_len) {
  AmsAddr addr = {this->_net_id, PORT};
  const auto read = reinterpret_cast<tc_ads_sync_read_req_ex>(GetProcAddress(this->_lib, TCADS_ADS_SYNC_READ_REQ_EX));

  unsigned long read_bytes;           // NOLINT
  const auto ret = read(this->_port,  // NOLINT
                        &addr, INDEX_GROUP, INDEX_OFFSET_BASE_READ, static_cast<uint32_t>(buffer_len), rx, &read_bytes);
  if (ret == 0) return Ok(true);

  std::stringstream ss;
  ss << "Error on reading data: " << std::hex << ret;
  return Err(ss.str());
}
}  // namespace autd::link
