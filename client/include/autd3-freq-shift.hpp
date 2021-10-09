// File: autd3.hpp
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

#include "autd3-freq-shift/controller.hpp"
#include "autd3-freq-shift/core/gain.hpp"
#include "autd3-freq-shift/core/hardware_defined.hpp"

namespace autd {
using core::DataArray;
using core::GainPtr;
using core::GeometryPtr;
using core::LinkPtr;
using core::Matrix4X4;
using core::Quaternion;
using core::Vector3;

using core::DEVICE_HEIGHT;
using core::DEVICE_WIDTH;
using core::FPGA_BASE_CLK_FREQ;
using core::FPGA_BASE_CLK_PERIOD_NS;
using core::NUM_TRANS_IN_UNIT;
using core::NUM_TRANS_X;
using core::NUM_TRANS_Y;
using core::TRANS_SPACING_MM;
using core::ULTRASOUND_FREQUENCY;
}  // namespace autd
