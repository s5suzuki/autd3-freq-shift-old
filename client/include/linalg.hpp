// File: linalg.hpp
// Project: include
// Created Date: 20/02/2021
// Author: Shun Suzuki
// -----
// Last Modified: 14/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include "autd_types.hpp"
#include "linalg/matrix.hpp"
#include "linalg/quaternion.hpp"
#include "linalg/vector.hpp"

namespace autd {

using Vector3 = utils::Vector3<Float>;
using Vector4 = utils::Vector4<Float>;
using Matrix4X4 = utils::Matrix4X4<Float>;
using Quaternion = utils::Quaternion<Float>;

template <typename V>
Vector3 ToVector3(const V& v) {
  Vector3 r;
  r(0) = v(0);
  r(1) = v(1);
  r(2) = v(2);
  return r;
}
}  // namespace autd
