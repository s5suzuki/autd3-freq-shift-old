// File: soem.cpp
// Project: examples
// Created Date: 19/05/2020
// Author: Shun Suzuki
// -----
// Last Modified: 26/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#include <iostream>

#include "autd3.hpp"
#include "runner.hpp"
#include "soemlink.hpp"

using namespace std;

string get_adapter_name() {
  size_t size;
  auto adapters = autd::link::SOEMLink::EnumerateAdapters(&size);
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
    auto cnt = autd::Controller::Create();

    // FPGA base clk frequency is 50MHz
    cnt->AddDevices(autd::Device::Create(Vector3(0, 0, 0), Vector3(0, 0, 0)), 1250);  // 50MHz/1250 = 40kHz
    cnt->AddDevices(autd::Device::Create(Vector3(0, 0, 0), Vector3(0, 0, 0)), 2500);  // 50MHz/1250 = 20kHz

    // If you have already recognized the EtherCAT adapter name, you can write it directly like below.
    // auto ifname = "\\Device\\NPF_{B5B631C6-ED16-4780-9C4C-3941AE8120A6}";
    const auto ifname = get_adapter_name();
    auto link = autd::link::SOEMLink::Create(ifname, cnt->num_devices());
    auto res = cnt->OpenWith(std::move(link));

    if (res.is_err()) {
      std::cerr << res.unwrap_err() << std::endl;
      return ENXIO;
    }
    if (!res.unwrap()) {
      std::cerr << "Failed to open." << std::endl;
      return ENXIO;
    }

    return run(std::move(cnt));
  } catch (exception& e) {
    std::cerr << e.what() << std::endl;
    return ENXIO;
  }
}
