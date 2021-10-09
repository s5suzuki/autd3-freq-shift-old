// File: logic.hpp
// Project: core
// Created Date: 11/05/2021
// Author: Shun Suzuki
// -----
// Last Modified: 10/10/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "gain.hpp"
#include "geometry.hpp"

namespace autd::core {
/**
 * \brief Hardware logic
 */
class Logic {
  static uint8_t get_id() {
    static std::atomic<uint8_t> id{0};

    id.fetch_add(0x01);
    uint8_t expected = 0xff;
    id.compare_exchange_weak(expected, 1);

    return id.load();
  }

 public:
  /**
   * \brief check if the data which have msg_id have been processed in the devices.
   * \param num_devices number of devices
   * \param msg_id message id
   * \param rx pointer to received data
   * \return whether the data have been processed
   */
  static bool is_msg_processed(const size_t num_devices, const uint8_t msg_id, const uint8_t* const rx) {
    size_t processed = 0;
    for (size_t dev = 0; dev < num_devices; dev++)
      if (const uint8_t proc_id = rx[dev * 2 + 1]; proc_id == msg_id) processed++;
    return processed == num_devices;
  }

  /**
   * \brief Pack header with COMMAND
   * \param cmd command
   * \param[out] data pointer to transmission data
   * \param[out] msg_id message id
   */
  static void pack_header(const COMMAND cmd, uint8_t* data, uint8_t* const msg_id) {
    auto* header = reinterpret_cast<GlobalHeader*>(data);
    *msg_id = get_id();
    header->msg_id = *msg_id;
    header->command = cmd;
  }

  /**
   * \brief Pack data body which contain duty data of each transducer.
   * \param gain Gain
   * \param[out] data pointer to transmission data
   * \param[out] size size to send
   */
  static void pack_duty_body(const GainPtr& gain, uint8_t* data, size_t* size) {
    const auto num_devices = gain != nullptr ? gain->duties().size() : 0;

    *size = sizeof(GlobalHeader) + sizeof(uint16_t) * NUM_TRANS_IN_UNIT * num_devices;
    if (gain == nullptr) return;

    auto* cursor = data + sizeof(GlobalHeader);
    const auto byte_size = NUM_TRANS_IN_UNIT * sizeof(uint16_t);
    for (size_t i = 0; i < num_devices; i++, cursor += byte_size) std::memcpy(cursor, gain->duties()[i].data(), byte_size);
  }

  /**
   * \brief Pack data body which contain phase data of each transducer.
   * \param gain Gain
   * \param[out] data pointer to transmission data
   * \param[out] size size to send
   */
  static void pack_phase_body(const GainPtr& gain, uint8_t* data, size_t* size) {
    const auto num_devices = gain != nullptr ? gain->phases().size() : 0;

    *size = sizeof(GlobalHeader) + sizeof(uint16_t) * NUM_TRANS_IN_UNIT * num_devices;
    if (gain == nullptr) return;

    auto* cursor = data + sizeof(GlobalHeader);
    const auto byte_size = NUM_TRANS_IN_UNIT * sizeof(uint16_t);
    for (size_t i = 0; i < num_devices; i++, cursor += byte_size) std::memcpy(cursor, gain->phases()[i].data(), byte_size);
  }

  /**
   * \brief Pack frequency body
   * \param geometry Geometry
   * \param[out] data pointer to transmission data
   * \param[out] size size to send
   */
  static void pack_freq_body(const GeometryPtr& geometry, uint8_t* data, size_t* size) {
    const auto num_devices = geometry->num_devices();

    *size = sizeof(GlobalHeader) + sizeof(uint16_t) * NUM_TRANS_IN_UNIT * num_devices;

    auto* cursor = reinterpret_cast<uint16_t*>(data + sizeof(GlobalHeader));
    for (size_t i = 0; i < num_devices; i++, cursor += NUM_TRANS_IN_UNIT) cursor[0] = geometry->freq_cycle(i);
  }
};
}  // namespace autd::core
