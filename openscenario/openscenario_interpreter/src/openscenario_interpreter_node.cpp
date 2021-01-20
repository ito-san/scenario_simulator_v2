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

#include <glog/logging.h>
#include <openscenario_interpreter/openscenario_interpreter.hpp>

#include <cstdlib>
#include <memory>

int main(int argc, char ** argv)
{
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  rclcpp::init(argc, argv);

  // rclcpp::executors::MultiThreadedExecutor executor {};
  rclcpp::executors::SingleThreadedExecutor executor {};

  rclcpp::NodeOptions options {};

  auto node {
    std::make_shared<openscenario_interpreter::Interpreter>(options)
  };

  executor.add_node((*node).get_node_base_interface());

  executor.spin();

  rclcpp::shutdown();

  return EXIT_SUCCESS;
}