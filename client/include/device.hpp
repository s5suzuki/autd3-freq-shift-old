// File: geometry.hpp
// Project: include
// Created Date: 11/04/2018
// Author: Shun Suzuki
// -----
// Last Modified: 15/04/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2018-2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <memory>
#include <utility>

#include "autd_types.hpp"
#include "linalg.hpp"

namespace autd {

struct Device {
  static Device Create(const Vector3& position, const Quaternion& quaternion) {
    const auto transform_matrix = Matrix4X4::Translation(position, quaternion);
    const auto x_direction = quaternion * Vector3(1, 0, 0);
    const auto y_direction = quaternion * Vector3(0, 1, 0);
    const auto z_direction = quaternion * Vector3(0, 0, 1);

    auto global_trans_positions = std::make_unique<Vector3[]>(NUM_TRANS_IN_UNIT);

    auto index = 0;
    for (size_t y = 0; y < NUM_TRANS_Y; y++)
      for (size_t x = 0; x < NUM_TRANS_X; x++)
        if (!IsMissingTransducer(x, y)) {
          const auto local_pos = Vector4(static_cast<Float>(x) * TRANS_SIZE_MM, static_cast<Float>(y) * TRANS_SIZE_MM, 0, 1);
          const auto global_pos = transform_matrix * local_pos;
          global_trans_positions[index++] = ToVector3(global_pos);
        }

    return Device(x_direction, y_direction, z_direction, std::move(global_trans_positions));
  }

  static Device Create(const Vector3& position, const Vector3& euler_angles) {
    const auto quaternion = Quaternion::AngleAxis(euler_angles.x(), Vector3::UnitZ()) * Quaternion::AngleAxis(euler_angles.y(), Vector3::UnitY()) *
                            Quaternion::AngleAxis(euler_angles.z(), Vector3::UnitZ());

    return Create(position, quaternion);
  }

  Vector3 x_direction() { return this->_x_direction; }
  Vector3 y_direction() { return this->_y_direction; }
  Vector3 z_direction() { return this->_z_direction; }
  std::shared_ptr<Vector3[]> global_trans_positions() { return this->_global_trans_positions; }

 private:
  Device(Vector3 x_direction, Vector3 y_direction, Vector3 z_direction, std::shared_ptr<Vector3[]> global_trans_positions)
      : _x_direction(x_direction),
        _y_direction(y_direction),
        _z_direction(z_direction),
        _global_trans_positions(std::move(_global_trans_positions)) {}

  Vector3 _x_direction;
  Vector3 _y_direction;
  Vector3 _z_direction;
  std::shared_ptr<Vector3[]> _global_trans_positions;
};
}  // namespace autd
