// File: controller.hpp
// Project: include
// Created Date: 14/04/2021
// Author: Shun Suzuki
// -----
// Last Modified: 09/10/2021
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

#include "autd3-freq-shift/core/ec_config.hpp"
#include "autd3-freq-shift/core/firmware_version.hpp"
#include "autd3-freq-shift/core/gain.hpp"
#include "autd3-freq-shift/core/geometry.hpp"
#include "autd3-freq-shift/core/link.hpp"
#include "autd3-freq-shift/core/logic.hpp"

namespace autd {

class Controller;
using ControllerPtr = std::unique_ptr<Controller>;

class Controller {
 public:
  static ControllerPtr create() { return std::make_unique<Controller>(); }

  Controller() : _check_ack(false), _tx_buf(nullptr), _rx_buf(nullptr), _link(nullptr), _geometry(std::make_unique<core::Geometry>()) {}
  ~Controller() {
    try {
      this->close();
    } catch (...) {
    }
  }

  void open(core::LinkPtr link) {
    this->close();

    this->_tx_buf = std::make_unique<uint8_t[]>(this->_geometry->num_devices() * core::EC_OUTPUT_FRAME_SIZE);
    this->_rx_buf = std::make_unique<uint8_t[]>(this->_geometry->num_devices() * core::EC_INPUT_FRAME_SIZE);

    core::LinkConfiguration config;
    for (size_t i = 0; i < _geometry->num_devices(); i++) config.freq_cycles.emplace_back(_geometry->freq_cycle(i));

    link->open(config);
    this->_link = std::move(link);
  }

  bool close() {
    if (this->_link == nullptr) return true;
    if (!this->_link->is_open()) return true;
    if (!this->stop()) return false;
    if (!this->clear()) return false;

    this->_link->close();
    this->_link = nullptr;
    this->_tx_buf = nullptr;
    this->_rx_buf = nullptr;

    return true;
  }

  bool is_open() const { return this->_link != nullptr && this->_link->is_open(); }
  core::GeometryPtr& geometry() noexcept { return this->_geometry; }

  bool clear() { return send_header(core::COMMAND::CLEAR); }

  bool stop() { return this->send(core::Gain::create()); }

  bool set_frequency() {
    uint8_t msg_id;
    core::Logic::pack_header(core::COMMAND::ULTRASOUND_CYCLE_CNT, &this->_tx_buf[0], &msg_id);
    size_t size = 0;
    core::Logic::pack_freq_body(this->_geometry, &this->_tx_buf[0], &size);
    this->_link->send(&this->_tx_buf[0], size);
    return wait_msg_processed(msg_id, 200);
  }

  bool send(const core::GainPtr& gain) {
    if (gain != nullptr) gain->build(this->_geometry);

    uint8_t msg_id;
    size_t size = 0;
    core::Logic::pack_header(core::COMMAND::WRITE_DUTY, &this->_tx_buf[0], &msg_id);
    core::Logic::pack_duty_body(gain, &this->_tx_buf[0], &size);
    this->_link->send(&this->_tx_buf[0], size);
    if (!wait_msg_processed(msg_id, 200)) return false;

    core::Logic::pack_header(core::COMMAND::WRITE_PHASE, &this->_tx_buf[0], &msg_id);
    core::Logic::pack_phase_body(gain, &this->_tx_buf[0], &size);
    this->_link->send(&this->_tx_buf[0], size);
    return wait_msg_processed(msg_id, 200);
  }

  std::vector<FirmwareInfo> firmware_info_list() {
    auto concat_byte = [](const uint8_t high, const uint16_t low) { return static_cast<uint16_t>(static_cast<uint16_t>(high) << 8 | low); };

    std::vector<FirmwareInfo> infos;

    const auto num_devices = this->_geometry->num_devices();
    const auto check_ack = this->_check_ack;
    this->_check_ack = true;

    std::vector<uint16_t> cpu_versions(num_devices);
    if (const auto res = send_header(core::COMMAND::READ_CPU_VER_LSB); !res) return infos;
    for (size_t i = 0; i < num_devices; i++) cpu_versions[i] = this->_rx_buf[2 * i];
    if (const auto res = send_header(core::COMMAND::READ_CPU_VER_MSB); !res) return infos;
    for (size_t i = 0; i < num_devices; i++) cpu_versions[i] = concat_byte(this->_rx_buf[2 * i], cpu_versions[i]);

    std::vector<uint16_t> fpga_versions(num_devices);
    if (const auto res = send_header(core::COMMAND::READ_FPGA_VER_LSB); !res) return infos;
    for (size_t i = 0; i < num_devices; i++) fpga_versions[i] = this->_rx_buf[2 * i];
    if (const auto res = send_header(core::COMMAND::READ_FPGA_VER_MSB); !res) return infos;
    for (size_t i = 0; i < num_devices; i++) fpga_versions[i] = concat_byte(this->_rx_buf[2 * i], fpga_versions[i]);

    this->_check_ack = check_ack;

    for (size_t i = 0; i < num_devices; i++) infos.emplace_back(FirmwareInfo(static_cast<uint16_t>(i), cpu_versions[i], fpga_versions[i]));
    return infos;
  }

 private:
  bool send_header(const core::COMMAND cmd) const {
    constexpr auto send_size = sizeof(core::GlobalHeader);
    uint8_t msg_id = 0;
    core::Logic::pack_header(cmd, &_tx_buf[0], &msg_id);
    _link->send(&_tx_buf[0], send_size);
    return wait_msg_processed(msg_id, 200);
  }

  bool wait_msg_processed(const uint8_t msg_id, const size_t max_trial) const {
    if (!this->_check_ack) return true;
    const auto num_devices = this->_geometry->num_devices();
    const auto buffer_len = num_devices * core::EC_INPUT_FRAME_SIZE;
    for (size_t i = 0; i < max_trial; i++) {
      this->_link->read(&_rx_buf[0], buffer_len);
      if (core::Logic::is_msg_processed(num_devices, msg_id, &_rx_buf[0])) return true;

      auto wait = static_cast<size_t>(std::ceil(core::EC_TRAFFIC_DELAY * 1000.0 / core::EC_DEVICE_PER_FRAME * static_cast<double>(num_devices)));
      std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    }

    return false;
  }

  bool _check_ack;

  std::unique_ptr<uint8_t[]> _tx_buf;
  std::unique_ptr<uint8_t[]> _rx_buf;

  core::LinkPtr _link;
  core::GeometryPtr _geometry;
};
}  // namespace autd
