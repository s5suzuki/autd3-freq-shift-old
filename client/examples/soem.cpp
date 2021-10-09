// File: soem.cpp
// Project: examples
// Created Date: 19/05/2020
// Author: Shun Suzuki
// -----
// Last Modified: 10/10/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#include "autd3-freq-shift/link/soem.hpp"

#include <iostream>

#include "autd3-freq-shift.hpp"
#include "autd3-freq-shift/core/utils.hpp"

class FocalPointGain final : public autd::core::Gain {
 public:
  static autd::GainPtr create(const autd::Vector3& point) { return std::make_shared<FocalPointGain>(std::forward<const autd::Vector3>(point)); }

  void calc(const autd::GeometryPtr& geometry) override {
    for (size_t dev_idx = 0; dev_idx < geometry->num_devices(); dev_idx++) {
      const uint16_t freq_cycle = geometry->freq_cycle(dev_idx);
      const double wavenum = 2.0 * M_PI / geometry->wavelength(dev_idx);
      for (size_t i = 0; i < autd::NUM_TRANS_IN_UNIT; i++) {
        const auto& trp = geometry->position(dev_idx, i);
        const auto dist = (trp - this->_point).norm();
        const auto phase = autd::core::Utilities::to_phase(dist * wavenum, freq_cycle);
        this->_duties[dev_idx][i] = autd::core::Utilities::to_duty(1.0, freq_cycle);
        this->_phases[dev_idx][i] = phase;
      }
    }
  }

  explicit FocalPointGain(const autd::Vector3& point) : Gain(), _point(std::forward<const autd::Vector3>(point)) {}

 private:
  autd::Vector3 _point = autd::Vector3::Zero();
};

std::string get_adapter_name() {
  size_t i = 0;
  const auto adapters = autd::link::SOEM::enumerate_adapters();
  for (auto&& [desc, name] : adapters) std::cout << "[" << i++ << "]: " << desc << ", " << name << std::endl;

  std::cout << "Choose number: ";
  std::string in;
  getline(std::cin, in);
  std::stringstream s(in);
  if (const auto empty = in == "\n"; !(s >> i) || i >= adapters.size() || empty) return "";

  return adapters[i].name;
}

int main() try {
  const auto cnt = autd::Controller::create();

  cnt->geometry()->sound_speed() = 340e3;  // mm/s, wavelength will be sound_speed/frequency, i.e., sound_speed/(200MHz/freq_cycle)

  // FPGA base clk frequency is 200MHz
  cnt->geometry()->add_device(autd::Vector3(0, 0, 0), autd::Vector3(0, 0, 0), 5000);  // 200MHz/5000 = 40kHz
  cnt->geometry()->add_device(autd::Vector3(0, 0, 0), autd::Vector3(0, 0, 0), 4999);  // 200MHz/4999 ~ 40.008kHz

  const auto ifname = get_adapter_name();
  auto link = autd::link::SOEM::create(ifname, cnt->geometry()->num_devices());

  cnt->open(std::move(link));

  cnt->clear();
  cnt->set_frequency();  // Set ultrasound frequency

  const auto firm_info_list = cnt->firmware_info_list();
  for (auto&& firm_info : firm_info_list) std::cout << firm_info << std::endl;

  const auto point =
      autd::Vector3(autd::TRANS_SPACING_MM * ((autd::NUM_TRANS_X - 1) / 2.0), autd::TRANS_SPACING_MM * ((autd::NUM_TRANS_Y - 1) / 2.0), 150.0);
  const auto g = FocalPointGain::create(point);
  cnt->send(g);

  std::cout << "press any key to finish..." << std::endl;
  std::cin.ignore();

  cnt->close();

  return 0;
} catch (std::exception& e) {
  std::cerr << e.what() << std::endl;
  return ENXIO;
}
