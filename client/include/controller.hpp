// File: controller.hpp
// Project: include
// Created Date: 14/04/2021
// Author: Shun Suzuki
// -----
// Last Modified: 26/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "consts.hpp"
#include "device.hpp"
#include "ec_config.hpp"
#include "firmware_version.hpp"
#include "gain.hpp"
#include "link.hpp"

using std::unique_ptr;

namespace autd {

class Controller;
using ControllerPtr = unique_ptr<Controller>;

constexpr uint8_t CMD_OP = 0x00;
constexpr uint8_t CMD_READ_CPU_VER_LSB = 0x02;
constexpr uint8_t CMD_READ_CPU_VER_MSB = 0x03;
constexpr uint8_t CMD_READ_FPGA_VER_LSB = 0x04;
constexpr uint8_t CMD_READ_FPGA_VER_MSB = 0x05;
constexpr uint8_t CMD_CLEAR = 0x09;

class Controller {
 public:
  static ControllerPtr Create() { return std::make_unique<Controller>(); }

  Controller() : _tx_capacity(0), _tx_data(nullptr), _rx_capacity(0), _rx_data(nullptr), _link(nullptr), _devices(), _freq_cycles() {}

  void AddDevices(Device device, uint16_t freq_cycle = FPGA_BASE_CLK_FREQ / 40000) {
    this->_devices.emplace_back(std::move(device));
    this->_freq_cycles.emplace_back(freq_cycle);
  }

  [[nodiscard]] size_t num_devices() { return this->_devices.size(); }

  [[nodiscard]] Result<bool, std::string> OpenWith(LinkPtr link) {
    this->_link = std::move(link);
    return this->_link->Open(LinkConfiguration{_freq_cycles});
  }

  [[nodiscard]] Result<bool, std::string> Close() {
    if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);
    return this->_link->Close();
  }

  Result<bool, std::string> Clear() { return SendHeaderBlocking(CMD_CLEAR, 200); }

  Result<bool, std::string> Stop() { return SendBlocking(NullGain::Create()); }

  Result<bool, std::string> SendBlocking(GainPtr gain) {
    size_t size;

    if (gain != nullptr) {
      auto res = gain->Build(this->_devices);
      if (res.is_err()) return res;
    }

    this->PackBody(gain, &size);
    return this->SendBlocking(size, 200);
  }

  Result<bool, std::string> Send(GainPtr gain) {
    size_t size;

    if (gain != nullptr) {
      auto res = gain->Build(this->_devices);
      if (res.is_err()) return res;
    }

    this->PackBody(gain, &size);
    return this->SendData(size);
  }

  [[nodiscard]] Result<std::vector<FirmwareInfo>, std::string> firmware_info_list() {
    const auto size = _devices.size();

    std::vector<uint16_t> cpu_versions(size);

    auto res = this->SendHeaderBlocking(CMD_READ_CPU_VER_LSB, 52000);
    if (res.is_err()) return Err(res.unwrap_err());
    for (size_t i = 0; i < size; i++) cpu_versions[i] = _rx_data[2 * i];

    res = this->SendHeaderBlocking(CMD_READ_CPU_VER_MSB, 200);
    if (res.is_err()) return Err(res.unwrap_err());
    for (size_t i = 0; i < size; i++) cpu_versions[i] = ConcatByte(_rx_data[2 * i], cpu_versions[i]);

    std::vector<uint16_t> fpga_versions(size);

    res = this->SendHeaderBlocking(CMD_READ_FPGA_VER_LSB, 200);
    for (size_t i = 0; i < size; i++) fpga_versions[i] = _rx_data[2 * i];

    res = this->SendHeaderBlocking(CMD_READ_FPGA_VER_MSB, 200);
    if (res.is_err()) return Err(res.unwrap_err());

    for (size_t i = 0; i < size; i++) fpga_versions[i] = ConcatByte(_rx_data[2 * i], fpga_versions[i]);

    std::vector<FirmwareInfo> infos;
    for (size_t i = 0; i < size; i++) infos.emplace_back(FirmwareInfo(static_cast<uint16_t>(i), cpu_versions[i], fpga_versions[i]));

    return Ok(std::move(infos));
  }

 private:
  Result<bool, std::string> SendHeaderBlocking(const uint8_t cmd, const size_t trial) {
    const auto size = sizeof(RxGlobalHeader);

    reserve_tx();
    auto* header = reinterpret_cast<RxGlobalHeader*>(&_tx_data[0]);
    header->msg_id = get_id();
    header->cmd = cmd;

    return this->SendBlocking(size, trial);
  }

  [[nodiscard]] Result<bool, std::string> SendBlocking(const size_t size, const size_t trial) {
    const auto msg_id = _tx_data[0];

    auto res = this->SendData(size);
    if (res.is_err()) return res;

    return this->WaitMsgProcessed(msg_id, trial);
  }

  Result<bool, std::string> SendData(const size_t size) const {
    if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);
    return this->_link->Send(size, _tx_data.get());
  }

  Result<bool, std::string> ReadData(const size_t size) const {
    if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);
    return this->_link->Read(_rx_data.get(), size);
  }

  Result<bool, std::string> WaitMsgProcessed(const uint8_t msg_id, const size_t max_trial) {
    if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);

    const auto num_dev = this->_devices.size();
    const auto buffer_len = num_dev * EC_INPUT_FRAME_SIZE;

    reserve_rx();
    for (size_t i = 0; i < max_trial; i++) {
      auto res = this->ReadData(buffer_len);
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

  void PackBody(const GainPtr& gain, size_t* size) {
    *size = sizeof(RxGlobalHeader);
    if (gain != nullptr) *size += sizeof(uint16_t) * NUM_TRANS_IN_UNIT * this->_devices.size();

    reserve_tx();

    auto* header = reinterpret_cast<RxGlobalHeader*>(&_tx_data[0]);
    header->msg_id = get_id();
    header->control_flags = RX_GLOBAL_CONTROL_FLAGS::NONE;
    header->cmd = CMD_OP;

    auto* cursor = &_tx_data[0] + sizeof(RxGlobalHeader);
    const auto byte_size = NUM_TRANS_IN_UNIT * sizeof(uint16_t);
    if (gain != nullptr) {
      for (size_t i = 0; i < _devices.size(); i++) {
        std::memcpy(cursor, &gain->data()[i].at(0), byte_size);
        cursor += byte_size;
      }
    }
  }

  static uint8_t get_id() {
    static std::atomic<uint8_t> id{0};

    uint8_t expected = 0xFF;
    if (!id.compare_exchange_weak(expected, 0)) {
      id.fetch_add(0x01);
    }
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
  std::vector<uint16_t> _freq_cycles;
};
}  // namespace autd
