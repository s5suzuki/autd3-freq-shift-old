// File: runner.hpp
// Project: examples
// Created Date: 19/05/2020
// Author: Shun Suzuki
// -----
// Last Modified: 16/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "autd3.hpp"
#include "examples/simple.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::function;
using std::pair;
using std::string;
using std::vector;

inline int run(autd::ControllerPtr cnt) {
  using f = function<void(autd::ControllerPtr&)>;
  vector<pair<f, string>> examples = {pair(f{simple_test}, "Single Focal Point Test")};

  cnt->Clear().unwrap();

  auto firm_info_list = cnt->firmware_info_list().unwrap();
  for (auto&& firm_info : firm_info_list) cout << firm_info << endl;
	
  while (true) {
    for (size_t i = 0; i < examples.size(); i++) cout << "[" << i << "]: " << examples[i].second << endl;
    cout << "[Others]: finish." << endl;

  	cout << "Choose number: ";
    string in;
    size_t idx = 0;
    getline(cin, in);
    if (std::stringstream s(in); !(s >> idx) || idx >= examples.size() || in == "\n") break;

    auto fn = examples[idx].first;
    fn(cnt);

    cout << "press any key to finish..." << endl;
    cin.ignore();

    cout << "finish." << endl;
    cnt->Stop().unwrap();
  }

  cnt->Clear().unwrap();
  cnt->Close().unwrap();

  return 0;
}
