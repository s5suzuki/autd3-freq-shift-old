// File: autd_logic.cpp
// Project: lib
// Created Date: 22/12/2020
// Author: Shun Suzuki
// -----
// Last Modified: 15/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#include "autd_logic.hpp"

#include <algorithm>
#include <cstring>
#include <thread>
#include <utility>

#include "ec_config.hpp"
#include "firmware_version.hpp"
#include "gain.hpp"
#include "link.hpp"
#include "modulation.hpp"
#include "pre_def.hpp"
#include "sequence.hpp"

namespace autd::internal {

using autd::NUM_TRANS_IN_UNIT;
using std::move;

AUTDLogic::AUTDLogic() {
  this->_geometry = Geometry::Create();
  this->_link = nullptr;

  this->_silent_mode = true;
  this->_seq_mode = false;

  this->_config = Configuration::GetDefaultConfiguration();
}

bool AUTDLogic::is_open() const { return this->_link != nullptr && this->_link->is_open(); }

GeometryPtr AUTDLogic::geometry() const noexcept { return this->_geometry; }

bool &AUTDLogic::silent_mode() noexcept { return this->_silent_mode; }

Result<bool, std::string> AUTDLogic::OpenWith(LinkPtr link) {
  this->_link = move(link);
  return this->_link->Open();
}

Result<bool, std::string> AUTDLogic::BuildGain(const GainPtr &gain) {
  if (gain == nullptr) return Ok(false);

  this->_seq_mode = false;
  gain->SetGeometry(this->_geometry);
  return gain->Build();
}

Result<bool, std::string> AUTDLogic::BuildModulation(const ModulationPtr &mod) const {
  if (mod == nullptr) return Ok(false);

  return mod->Build(this->_config);
}

Result<bool, std::string> AUTDLogic::Send(const GainPtr &gain, const ModulationPtr &mod) {
  if (gain != nullptr) this->_seq_mode = false;

  size_t body_size = 0;
  uint8_t msg_id = 0;
  const auto body = this->MakeBody(gain, mod, &body_size, &msg_id);
  return this->SendData(body_size, &body[0]);
}

Result<bool, std::string> AUTDLogic::SendBlocking(const GainPtr &gain, const ModulationPtr &mod) {
  if (gain != nullptr) this->_seq_mode = false;

  size_t body_size = 0;
  uint8_t msg_id = 0;
  const auto body = this->MakeBody(gain, mod, &body_size, &msg_id);
  auto res = this->SendData(body_size, &body[0]);
  if (res.is_err()) return res;
  return WaitMsgProcessed(msg_id);
}

Result<bool, std::string> AUTDLogic::SendData(const size_t size, const uint8_t *data) const {
  if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);

  return this->_link->Send(size, data);
}

Result<bool, std::string> AUTDLogic::Synchronize(const Configuration config) {
  this->_config = config;
  size_t size = 0;
  auto res = this->MakeCalibBody(config, &size);
  if (res.is_err()) return Err(res.unwrap_err());

  return this->SendBlocking(size, &res.unwrap()[0], 5000);
}

Result<bool, std::string> AUTDLogic::SynchronizeSeq() {
  std::vector<uint16_t> laps;
  for (size_t i = 0; i < this->_rx_data.size() / 2; i++) {
    const auto lap_raw = static_cast<uint16_t>(_rx_data[2 * i + 1]) << 8 | _rx_data[2 * i];
    laps.emplace_back(lap_raw & 0x03FF);
  }

  std::vector<uint16_t> diffs;
  diffs.reserve(laps.size());
  auto minimum = *std::min_element(laps.begin(), laps.end());
  for (auto lap : laps) diffs.emplace_back(lap - minimum);

  const auto diff_max = *std::max_element(diffs.begin(), diffs.end());
  if (diff_max == 0) return Ok(true);

  if (diff_max > 500) {
    for (auto &lap : laps) lap = lap < 500 ? lap + 1000 : lap;

    minimum = *std::min_element(laps.begin(), laps.end());
    for (size_t i = 0; i < laps.size(); i++) diffs[i] = laps[i] - minimum;
  }

  size_t body_size = 0;
  const auto calib_body = this->MakeCalibSeqBody(diffs, &body_size);
  auto res = this->SendData(body_size, &calib_body[0]);
  if (res.is_err()) return res;

  return this->WaitMsgProcessed(0xE0, 200, 0xE0);
}

Result<bool, std::string> AUTDLogic::Clear() {
  this->_config = Configuration::GetDefaultConfiguration();

  const auto size = sizeof(RxGlobalHeader);
  const auto body = std::make_unique<uint8_t[]>(size);

  auto *header = reinterpret_cast<RxGlobalHeader *>(&body[0]);
  header->msg_id = CMD_CLEAR;
  header->command = CMD_CLEAR;

  return this->SendBlocking(size, &body[0], 200);
}

Result<bool, std::string> AUTDLogic::Close() {
  if (this->_link == nullptr || !this->_link->is_open()) return Ok(false);

  auto clear_result = this->Clear();

  auto close_result = this->_link->Close();
  if (close_result.is_err()) return close_result;

  this->_link = nullptr;
  return Ok(clear_result.unwrap_or(false) && close_result.unwrap());
}

unique_ptr<uint8_t[]> AUTDLogic::MakeBody(const GainPtr &gain, const ModulationPtr &mod, size_t *const size, uint8_t *const send_msg_id) const {
  const auto num_devices = gain != nullptr ? gain->geometry()->num_devices() : 0;

  *size = sizeof(RxGlobalHeader) + sizeof(uint16_t) * NUM_TRANS_IN_UNIT * num_devices;
  auto body = std::make_unique<uint8_t[]>(*size);

  auto *header = reinterpret_cast<RxGlobalHeader *>(&body[0]);
  *send_msg_id = get_id();
  header->msg_id = *send_msg_id;
  header->control_flags = 0;
  header->mod_size = 0;
  header->command = CMD_OP;

  if (this->_seq_mode) header->control_flags |= SEQ_MODE;
  if (this->_silent_mode) header->control_flags |= SILENT;

  if (mod != nullptr) {
    const auto mod_size = static_cast<uint8_t>(std::clamp(mod->buffer.size() - mod->sent(), size_t{0}, MOD_FRAME_SIZE));
    header->mod_size = mod_size;
    if (mod->sent() == 0) header->control_flags |= LOOP_BEGIN;
    if (mod->sent() + mod_size >= mod->buffer.size()) header->control_flags |= LOOP_END;

    std::memcpy(header->mod, &mod->buffer[mod->sent()], mod_size);
    mod->sent() += mod_size;
  }

  auto *cursor = &body[0] + sizeof(RxGlobalHeader);
  const auto byte_size = NUM_TRANS_IN_UNIT * sizeof(uint16_t);
  if (gain != nullptr) {
    for (size_t i = 0; i < gain->geometry()->num_devices(); i++) {
      std::memcpy(cursor, &gain->data()[i].at(0), byte_size);
      cursor += byte_size;
    }
  }
  return body;
}

unique_ptr<uint8_t[]> AUTDLogic::MakeBody(const SequencePtr &seq, size_t *const size, uint8_t *const send_msg_id) const {
  const auto num_devices = this->_geometry->num_devices();

  *size = sizeof(RxGlobalHeader) + sizeof(uint16_t) * NUM_TRANS_IN_UNIT * num_devices;
  auto body = std::make_unique<uint8_t[]>(*size);

  auto *header = reinterpret_cast<RxGlobalHeader *>(&body[0]);
  *send_msg_id = get_id();
  header->msg_id = *send_msg_id;
  header->control_flags = SEQ_MODE;
  header->command = CMD_SEQ_MODE;
  header->mod_size = 0;

  if (this->_silent_mode) header->control_flags |= SILENT;

  const auto send_size = static_cast<uint16_t>(std::clamp(seq->control_points().size() - seq->sent(), size_t{0}, size_t{40}));
  header->seq_size = send_size;
  header->seq_div = seq->sampling_frequency_division();

  if (seq->sent() == 0) {
    header->control_flags |= SEQ_BEGIN;
  }
  if (seq->sent() + send_size >= seq->control_points().size()) {
    header->control_flags |= SEQ_END;
  }

  auto *cursor = &body[0] + sizeof(RxGlobalHeader);
  const auto fixed_num_unit = _geometry->wavelength() / 256;
  for (size_t device = 0; device < num_devices; device++) {
    std::vector<uint8_t> foci;
    foci.reserve(static_cast<size_t>(send_size) * 10);

    for (size_t i = 0; i < send_size; i++) {
      auto v64 = this->_geometry->local_position(device, seq->control_points()[seq->sent() + i]);
      const auto x = static_cast<uint32_t>(static_cast<int32_t>(v64.x() / fixed_num_unit));
      const auto y = static_cast<uint32_t>(static_cast<int32_t>(v64.y() / fixed_num_unit));
      const auto z = static_cast<uint32_t>(static_cast<int32_t>(v64.z() / fixed_num_unit));
      foci.emplace_back(static_cast<uint8_t>(x & 0x000000FF));
      foci.emplace_back(static_cast<uint8_t>((x & 0x0000FF00) >> 8));
      foci.emplace_back(static_cast<uint8_t>((x & 0x80000000) >> 24 | (x & 0x007F0000) >> 16));
      foci.emplace_back(static_cast<uint8_t>(y & 0x000000FF));
      foci.emplace_back(static_cast<uint8_t>((y & 0x0000FF00) >> 8));
      foci.emplace_back(static_cast<uint8_t>((y & 0x80000000) >> 24 | (y & 0x007F0000) >> 16));
      foci.emplace_back(static_cast<uint8_t>(z & 0x000000FF));
      foci.emplace_back(static_cast<uint8_t>((z & 0x0000FF00) >> 8));
      foci.emplace_back(static_cast<uint8_t>((z & 0x80000000) >> 24 | (z & 0x007F0000) >> 16));
      foci.emplace_back(0xFF);  // amp
    }
    std::memcpy(cursor, &foci[0], foci.size());
    cursor += NUM_TRANS_IN_UNIT * 2;
  }

  seq->sent() += send_size;
  return body;
}

Result<unique_ptr<uint8_t[]>, std::string> AUTDLogic::MakeCalibBody(const Configuration config, size_t *const size) {
  this->_config = config;

  const auto num_devices = this->_geometry->num_devices();
  *size = sizeof(RxGlobalHeader) + sizeof(uint16_t) * NUM_TRANS_IN_UNIT * num_devices;
  auto body = std::make_unique<uint8_t[]>(*size);

  auto *header = reinterpret_cast<RxGlobalHeader *>(&body[0]);
  header->msg_id = CMD_INIT_REF_CLOCK;
  header->command = CMD_INIT_REF_CLOCK;

  const auto mod_sampling_freq = static_cast<uint32_t>(_config.mod_sampling_freq());
  const auto mod_buf_size = static_cast<uint32_t>(_config.mod_buf_size());

  if (mod_buf_size < mod_sampling_freq) return Err(std::string("Modulation buffer size must be not less than sampling frequency"));

  const auto mod_idx_shift = Log2U(MOD_SAMPLING_FREQ_BASE / mod_sampling_freq);
  const auto ref_clk_cyc_shift = Log2U(mod_buf_size / mod_sampling_freq);

  auto *cursor = reinterpret_cast<uint16_t *>(&body[0] + sizeof(RxGlobalHeader));
  for (size_t i = 0; i < num_devices; i++) {
    cursor[0] = mod_idx_shift;
    cursor[1] = ref_clk_cyc_shift;
    cursor += NUM_TRANS_IN_UNIT;
  }

  return Ok(std::move(body));
}

unique_ptr<uint8_t[]> AUTDLogic::MakeCalibSeqBody(const std::vector<uint16_t> &comps, size_t *const size) const {
  *size = sizeof(RxGlobalHeader) + sizeof(uint16_t) * NUM_TRANS_IN_UNIT * comps.size();
  auto body = std::make_unique<uint8_t[]>(*size);

  auto *header = reinterpret_cast<RxGlobalHeader *>(&body[0]);
  header->msg_id = CMD_CALIB_SEQ_CLOCK;
  header->control_flags = 0;
  header->command = CMD_CALIB_SEQ_CLOCK;

  auto *cursor = reinterpret_cast<uint16_t *>(&body[sizeof(RxGlobalHeader)]);
  for (auto comp : comps) {
    *cursor = comp;
    cursor += NUM_TRANS_IN_UNIT;
  }

  return body;
}
}  // namespace autd::internal
