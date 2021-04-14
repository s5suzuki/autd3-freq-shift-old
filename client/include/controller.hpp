// File: controller.hpp
// Project: include
// Created Date: 14/04/2021
// Author: Shun Suzuki
// -----
// Last Modified: 14/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

namespace autd {

using std::unique_ptr;

constexpr uint8_t CMD_OP = 0x00;
constexpr uint8_t CMD_READ_CPU_VER_LSB = 0x02;
constexpr uint8_t CMD_READ_CPU_VER_MSB = 0x03;
constexpr uint8_t CMD_READ_FPGA_VER_LSB = 0x04;
constexpr uint8_t CMD_READ_FPGA_VER_MSB = 0x05;
constexpr uint8_t CMD_CLEAR = 0x09;

class AUTDLogic {
 public:
  AUTDLogic();

  [[nodiscard]] Result<bool, std::string> OpenWith(LinkPtr link);

  [[nodiscard]] Result<bool, std::string> BuildGain(const GainPtr& gain);
  [[nodiscard]] Result<bool, std::string> BuildModulation(const ModulationPtr& mod) const;

  [[nodiscard]] Result<bool, std::string> Send(const GainPtr& gain, const ModulationPtr& mod);
  [[nodiscard]] Result<bool, std::string> SendBlocking(const GainPtr& gain, const ModulationPtr& mod);
  [[nodiscard]] Result<bool, std::string> SendBlocking(const SequencePtr& seq);
  [[nodiscard]] Result<bool, std::string> SendBlocking(size_t size, const uint8_t* data, size_t trial);
  [[nodiscard]] Result<bool, std::string> SendData(size_t size, const uint8_t* data) const;

  [[nodiscard]] Result<bool, std::string> WaitMsgProcessed(uint8_t msg_id, size_t max_trial = 200, uint8_t mask = 0xFF);
  [[nodiscard]] Result<bool, std::string> Synchronize(Configuration config);
  [[nodiscard]] Result<bool, std::string> SynchronizeSeq();
  [[nodiscard]] Result<bool, std::string> Clear();
  [[nodiscard]] Result<bool, std::string> Close();
  [[nodiscard]] Result<std::vector<FirmwareInfo>, std::string> firmware_info_list();

  unique_ptr<uint8_t[]> MakeBody(const GainPtr& gain, const ModulationPtr& mod, size_t* size, uint8_t* send_msg_id) const;
  unique_ptr<uint8_t[]> MakeBody(const SequencePtr& seq, size_t* size, uint8_t* send_msg_id) const;
  [[nodiscard]] Result<unique_ptr<uint8_t[]>, std::string> MakeCalibBody(Configuration config, size_t* size);
  unique_ptr<uint8_t[]> MakeCalibSeqBody(const std::vector<uint16_t>& comps, size_t* size) const;

 private:
  static uint8_t get_id() {
    static std::atomic<uint8_t> id{OP_MODE_MSG_ID_MIN - 1};

    id.fetch_add(0x01);
    uint8_t expected = OP_MODE_MSG_ID_MAX + 1;
    id.compare_exchange_weak(expected, OP_MODE_MSG_ID_MIN);

    return id.load();
  }

  static uint16_t Log2U(const uint32_t x) {
#ifdef _MSC_VER
    unsigned long n;         // NOLINT
    _BitScanReverse(&n, x);  // NOLINT
#else
    uint32_t n;
    n = 31 - __builtin_clz(x);
#endif
    return static_cast<uint16_t>(n);
  }
};
}  // namespace autd
