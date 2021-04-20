// Copyright 2015-2020 Tier IV, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AWAPI_ACCESSOR__AUTOWARE_HPP_
#define AWAPI_ACCESSOR__AUTOWARE_HPP_

#include <awapi_accessor/low_level_api.hpp>
#include <awapi_accessor/miscellaneous_api.hpp>
#include <awapi_accessor/utility/visibility.hpp>
#include <mutex>

namespace awapi
{
template <typename Node>
class ContinuousTransformBroadcaster
{
  tf2_ros::Buffer transform_buffer;

  tf2_ros::TransformBroadcaster transform_broadcaster;

  geometry_msgs::msg::TransformStamped current_transform;

  const rclcpp::TimerBase::SharedPtr timer;

public:
  const auto & setTransform(const geometry_msgs::msg::Pose & pose)
  {
    current_transform.header.stamp = static_cast<Node &>(*this).get_clock()->now();
    current_transform.header.frame_id = "map";
    current_transform.child_frame_id = "base_link";
    current_transform.transform.translation.x = pose.position.x;
    current_transform.transform.translation.y = pose.position.y;
    current_transform.transform.translation.z = pose.position.z;
    current_transform.transform.rotation = pose.orientation;

    return current_transform;
  }

  void updateTransform()
  {
    if (!current_transform.header.frame_id.empty() && !current_transform.child_frame_id.empty()) {
      current_transform.header.stamp = static_cast<Node &>(*this).get_clock()->now();
      return transform_broadcaster.sendTransform(current_transform);
    }
  }

  explicit ContinuousTransformBroadcaster()
  : transform_buffer(static_cast<Node &>(*this).get_clock()),
    transform_broadcaster(static_cast<Node *>(this)),
    timer(static_cast<Node &>(*this).create_wall_timer(
      std::chrono::milliseconds(5), [this]() { return updateTransform(); }))
  {
  }
};

class Autoware : public rclcpp::Node,
                 public ContinuousTransformBroadcaster<Autoware>,
                 public LowLevelAPI<Autoware>,
                 public MiscellaneousAPI<Autoware>
{
  std::mutex mutex;

public:
  template <typename... Ts>
  AWAPI_ACCESSOR_PUBLIC explicit constexpr Autoware(Ts &&... xs)
  : rclcpp::Node(std::forward<decltype(xs)>(xs)...)
  {
  }

  decltype(auto) lock() { return std::unique_lock<std::mutex>(mutex); }
};
}  // namespace awapi

#include <awapi_accessor/undefine_macro.hpp>

#endif  // AWAPI_ACCESSOR__AUTOWARE_HPP_
