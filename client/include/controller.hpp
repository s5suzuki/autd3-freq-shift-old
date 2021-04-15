// File: controller.hpp
// Project: include
// Created Date: 14/04/2021
// Author: Shun Suzuki
// -----
// Last Modified: 15/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace autd {

using std::unique_ptr;

constexpr uint8_t CMD_OP = 0x00;
constexpr uint8_t CMD_READ_CPU_VER_LSB = 0x02;
constexpr uint8_t CMD_READ_CPU_VER_MSB = 0x03;
constexpr uint8_t CMD_READ_FPGA_VER_LSB = 0x04;
constexpr uint8_t CMD_READ_FPGA_VER_MSB = 0x05;
constexpr uint8_t CMD_CLEAR = 0x09;

size_t _body_capacity;
std::unique_ptr<uint8_t[]> _body;
LinkPtr _link;
std::vector<Device> _devices;

static class Controller {
 public:
  Controller(LinkPtr link) : _tx_capacity(0), _tx_data(nullptr), _rx_capacity(0), _rx_data(nullptr), _link(nullptr), _devices() {}

  [[nodiscard]] Result<std::vector<FirmwareInfo>, std::string> get_firmware_info_list(LinkPtr link, size_t num_devices) {
    std::vector<FirmwareInfo> infos;
    auto make_header = [](const uint8_t command) {
      auto header_bytes = std::make_unique<uint8_t[]>(sizeof(RxGlobalHeader));
      auto* header = reinterpret_cast<RxGlobalHeader*>(&header_bytes[0]);
      header->msg_id = command;
      header->command = command;
      return header_bytes;
    };

    std::vector<uint16_t> cpu_versions(size);
    std::vector<uint16_t> fpga_versions(size);

    const auto send_size = sizeof(RxGlobalHeader);
    auto header = make_header(CMD_READ_CPU_VER_LSB);
    auto res = this->SendData(send_size, &header[0]);
    if (res.is_err()) return Err(res.unwrap_err());

    res = WaitMsgProcessed(CMD_READ_CPU_VER_LSB, 50);
    if (res.is_err()) return Err(res.unwrap_err());

    for (size_t i = 0; i < size; i++) cpu_versions[i] = _rx_data[2 * i];

    header = make_header(CMD_READ_CPU_VER_MSB);
    res = this->SendData(send_size, &header[0]);
    if (res.is_err()) return Err(res.unwrap_err());
    res = WaitMsgProcessed(CMD_READ_CPU_VER_MSB, 50);
    if (res.is_err()) return Err(res.unwrap_err());

    for (size_t i = 0; i < size; i++) cpu_versions[i] = ConcatByte(_rx_data[2 * i], cpu_versions[i]);

    header = make_header(CMD_READ_FPGA_VER_LSB);
    res = this->SendData(send_size, &header[0]);
    if (res.is_err()) return Err(res.unwrap_err());

    res = WaitMsgProcessed(CMD_READ_FPGA_VER_LSB, 50);
    if (res.is_err()) return Err(res.unwrap_err());

    for (size_t i = 0; i < size; i++) fpga_versions[i] = _rx_data[2 * i];

    header = make_header(CMD_READ_FPGA_VER_MSB);
    res = this->SendData(send_size, &header[0]);

    if (res.is_err()) return Err(res.unwrap_err());
    res = WaitMsgProcessed(CMD_READ_FPGA_VER_MSB, 50);
    if (res.is_err()) return Err(res.unwrap_err());

    for (size_t i = 0; i < size; i++) fpga_versions[i] = ConcatByte(_rx_data[2 * i], fpga_versions[i]);

    for (size_t i = 0; i < size; i++) {
      auto info = FirmwareInfo(static_cast<uint16_t>(i), cpu_versions[i], fpga_versions[i]);
      infos.emplace_back(info);
    }
    return Ok(std::move(infos));
  }

 private:
  [[nodiscard]] Result<bool, std::string> AUTDLogic::SendBlocking(const size_t size, const uint8_t* data, const size_t trial) {
    const auto msg_id = data[0];

    auto res = this->SendData(size, data);
    if (res.is_err()) return res;

    return this->WaitMsgProcessed(msg_id, trial);
  }

  Result<bool, std::string> AUTDLogic::SendData(const size_t size, const uint8_t* data) const {
    if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);
    return this->_link->Send(size, data);
  }

  Result<bool, std::string> AUTDLogic::ReadData(const size_t size, const uint8_t* data) const {
    if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);
    return this->_link->Send(size, data);
  }

  Result<bool, std::string> AUTDLogic::WaitMsgProcessed(const uint8_t msg_id, const size_t max_trial) {
    if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);

    const auto num_dev = this->_devices.size();
    const auto buffer_len = num_dev * EC_INPUT_FRAME_SIZE;

    reserve_rx();
    for (size_t i = 0; i < max_trial; i++) {
      auto res = this->_link->Read(&_rx_data[0], static_cast<uint32_t>(buffer_len));
      if (res.is_err()) return res;

      size_t processed = 0;
      for (size_t dev = 0; dev < num_dev; dev++) {
        const uint8_t proc_id = _rx_data[dev * 2 + 1];
        if (proc_id == msg_id) processed++;
      }

      if (processed == num_dev) return Ok(true);

      auto wait = static_cast<size_t>(std::ceil(static_cast<double>(EC_TRAFFIC_DELAY) * 1000 / EC_DEVICE_PER_FRAME * static_cast<double>(num_dev)));
      std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    }

    return Ok(false);
  }

  void PackBody(const GainPtr& gain, const ModulationPtr& mod, size_t* size, uint8_t* send_msg_id) {}

  static uint8_t get_id() {
    static std::atomic<uint8_t> id{OP_MODE_MSG_ID_MIN - 1};

    id.fetch_add(0x01);
    uint8_t expected = OP_MODE_MSG_ID_MAX + 1;
    id.compare_exchange_weak(expected, OP_MODE_MSG_ID_MIN);

    return id.load();
  }

  void reserve_rx() {
    const auto dev_num = _devices.size();
    const auto size = dev_num * EC_INPUT_FRAME_SIZE;
    if (size > _rx_capacity) {
      _rx_data = std::make_unique<uint8_t[]>(size);
      _rx_capacity = size;
    }
  }

  void reserve_tx() {
    const auto dev_num = _devices.size();
    const auto size = dev_num * EC_OUTPUT_FRAME_SIZE;
    if (_tx_data == nullptr || size > _tx_capacity) {
      _tx_data = std::make_unique<uint8_t[]>(size);
      _tx_capacity = size;
    }
  }

  inline uint16_t ConcatByte(const uint8_t high, const uint16_t low) { return static_cast<uint16_t>(static_cast<uint16_t>(high) << 8 | low); }

  size_t _tx_capacity;
  std::unique_ptr<uint8_t[]> _tx_data;
  size_t _rx_capacity;
  std::unique_ptr<uint8_t[]> _rx_data;

  LinkPtr _link;
  std::vector<Device> _devices;
};
}  // namespace autd
