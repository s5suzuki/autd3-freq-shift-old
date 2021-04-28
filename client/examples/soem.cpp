// File: soem.cpp
// Project: examples
// Created Date: 19/05/2020
// Author: Shun Suzuki
// -----
// Last Modified: 28/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#include <iostream>

#include "autd3.hpp"
#include "soemlink.hpp"

using namespace std;
using namespace autd;

constexpr Float ULTRASOUND_WAVELENGTH = 8.5f;

class FocalPointGain final : public Gain {
 public:
  static GainPtr Create(const Vector3& point) { return std::make_shared<FocalPointGain>(std::forward<const Vector3>(point)); }

  Result<bool, std::string> Build(std::vector<Device>& devices, std::vector<uint16_t>& freq_cycle) override {
    if (this->_built) return Ok(false);

    CheckAndInit(devices, &this->_duties, &this->_phases);

    for (size_t dev_idx = 0; dev_idx < devices.size(); dev_idx++) {
      const uint16_t duty = freq_cycle[dev_idx] >> 1;  // 50%
      const auto& tr_positions = devices[dev_idx].global_trans_positions();
      for (size_t i = 0; i < NUM_TRANS_IN_UNIT; i++) {
        const auto& trp = tr_positions[i];
        const auto dist = (trp - this->_point).norm();
        const auto f_phase = fmod(dist, ULTRASOUND_WAVELENGTH) / ULTRASOUND_WAVELENGTH;
        const auto phase = static_cast<uint16_t>(std::round(static_cast<float>(freq_cycle[dev_idx]) * (1 - f_phase)));
        this->_duties[dev_idx][i] = duty;
        this->_phases[dev_idx][i] = phase;
      }
    }

    this->_built = true;
    return Ok(true);
  }

  explicit FocalPointGain(const Vector3& point) : Gain(), _point(std::forward<const Vector3>(point)) {}

 private:
  Vector3 _point = Vector3::Zero();
};

string get_adapter_name() {
  size_t size;
  auto adapters = link::SOEMLink::EnumerateAdapters(&size);
  for (size_t i = 0; i < size; i++) {
    auto& [fst, snd] = adapters[i];
    cout << "[" << i << "]: " << fst << ", " << snd << endl;
  }

  int index;
  cout << "Choose number: ";
  cin >> index;
  cin.ignore();

  return adapters[index].second;
}

int main() {
  try {
    auto cnt = Controller::Create();

    // FPGA base clk frequency is 50MHz
    cnt->AddDevices(Device::Create(Vector3(0, 0, 0), Vector3(0, 0, 0)), 1250);  // 50MHz/1250 = 40kHz
    cnt->AddDevices(Device::Create(Vector3(0, 0, 0), Vector3(0, 0, 0)), 1200);  // 50MHz/1200 = 41.667kHz

    const auto ifname = get_adapter_name();
    auto link = link::SOEMLink::Create(ifname, cnt->num_devices());

    if (auto res = cnt->OpenWith(std::move(link)); res.is_err()) {
      std::cerr << res.unwrap_err() << std::endl;
      return ENXIO;
    }

    cnt->Clear().unwrap();
    cnt->SetFreqCycles().unwrap();  // Set ultrasound frequency

    auto firm_info_list = cnt->firmware_info_list().unwrap();
    for (auto&& firm_info : firm_info_list) cout << firm_info << endl;

    const auto center = Vector3(TRANS_SIZE_MM * ((NUM_TRANS_X - 1) / 2.0f), TRANS_SIZE_MM * ((NUM_TRANS_Y - 1) / 2.0f), 150.0f);
    const auto g = FocalPointGain::Create(center);
    cnt->SendBlocking(g).unwrap();

    cout << "press any key to finish..." << endl;
    cin.ignore();

    cnt->Close().unwrap();

    return 0;
  } catch (exception& e) {
    std::cerr << e.what() << std::endl;
    return ENXIO;
  }
}
