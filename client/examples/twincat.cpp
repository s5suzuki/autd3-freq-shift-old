// File: soem.cpp
// Project: examples
// Created Date: 19/05/2020
// Author: Shun Suzuki
// -----
// Last Modified: 21/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#include <iostream>

#include "autd3.hpp"
#include "runner.hpp"
#include "twincatlink.hpp"

using namespace std;

int main() {
  try {
    auto cnt = autd::Controller::Create();

    cnt->devices().emplace_back(autd::Device::Create(Vector3(0, 0, 0), Vector3(0, 0, 0)));
    cnt->devices().emplace_back(autd::Device::Create(Vector3(0, 0, 0), Vector3(0, 0, 0)));

    auto link = autd::link::TwinCATLink::Create();
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
