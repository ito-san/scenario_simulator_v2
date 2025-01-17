// Copyright 2015 TIER IV, Inc. All rights reserved.
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

#include <iterator>  // std::distance
#include <openscenario_interpreter/error.hpp>
#include <openscenario_interpreter/posix/fork_exec.hpp>
#include <openscenario_interpreter/reader/attribute.hpp>
#include <openscenario_interpreter/reader/content.hpp>
#include <openscenario_interpreter/regex/function_call_expression.hpp>
#include <openscenario_interpreter/simulator_core.hpp>
#include <openscenario_interpreter/syntax/custom_command_action.hpp>
#include <unordered_map>

namespace openscenario_interpreter
{
inline namespace syntax
{
CustomCommandAction::CustomCommandAction(const pugi::xml_node & node, const Scope & scope)
: Scope(scope),
  type(readAttribute<String>("type", node, local())),
  content(readContent<String>(node, local()))
{
}

auto CustomCommandAction::accomplished() noexcept -> bool { return true; }

auto CustomCommandAction::applyFaultInjectionAction(
  const std::vector<std::string> & events, const Scope &) -> int
{
  const auto now = node().now();

  auto makeFaultInjectionEvent = [](const auto & name) {
    tier4_simulation_msgs::msg::FaultInjectionEvent fault_injection_event;
    {
      fault_injection_event.level = tier4_simulation_msgs::msg::FaultInjectionEvent::ERROR;
      fault_injection_event.name = name;
    }

    return fault_injection_event;
  };

  auto makeFaultInjectionEvents = [&](const std::vector<std::string> & events) {
    tier4_simulation_msgs::msg::SimulationEvents simulation_events;
    {
      simulation_events.stamp = now;

      for (const auto & event : events) {
        simulation_events.fault_injection_events.push_back(makeFaultInjectionEvent(event));
      }
    }

    return simulation_events;
  };

  publisher().publish(makeFaultInjectionEvents(events));

  return events.size();
}

auto CustomCommandAction::debugError(const std::vector<std::string> &, const Scope &) -> int
{
  throw Error(__FILE__, ":", __LINE__);
}

auto CustomCommandAction::debugSegmentationFault(const std::vector<std::string> &, const Scope &)
  -> int
{
  return *reinterpret_cast<std::add_pointer<int>::type>(0);
}

auto CustomCommandAction::exitSuccess(const std::vector<std::string> &, const Scope &) -> int
{
  throw SpecialAction<EXIT_SUCCESS>();
}

auto CustomCommandAction::exitFailure(const std::vector<std::string> &, const Scope &) -> int
{
  throw SpecialAction<EXIT_FAILURE>();
}

auto CustomCommandAction::node() -> rclcpp::Node &
{
  static rclcpp::Node node{"custom_command_action", "simulation"};
  return node;
}

auto CustomCommandAction::publisher()
  -> rclcpp::Publisher<tier4_simulation_msgs::msg::SimulationEvents> &
{
  static auto publisher = node().create_publisher<tier4_simulation_msgs::msg::SimulationEvents>(
    "/simulation/events", rclcpp::QoS(1).reliable());
  return *publisher;
}

auto CustomCommandAction::run() noexcept -> void {}

auto CustomCommandAction::start() const -> void
{
  auto apply_walk_straight_action =
    [this](const std::vector<std::string> & actors, const Scope & scope) {
      for (const auto & actor : actors) {
        applyWalkStraightAction(actor);
      }

      for (const auto & actor : scope.actors) {
        applyWalkStraightAction(actor);
      }

      return scope.actors.size();
    };

  static const std::unordered_map<
    std::string, std::function<int(const std::vector<std::string> &, const Scope &)>>
    commands{
      std::make_pair("FaultInjectionAction", applyFaultInjectionAction),
      std::make_pair("WalkStraightAction", apply_walk_straight_action),
      std::make_pair("debugError", debugError),
      std::make_pair("debugSegmentationFault", debugSegmentationFault),  // DEPRECATED
      std::make_pair("exitFailure", exitFailure),
      std::make_pair("exitSuccess", exitSuccess),
      std::make_pair("test", test),
    };

  std::smatch result{};

  if (type == ":") {
    return;
  } else if (
    std::regex_match(type, result, FunctionCallExpression::pattern()) and
    commands.find(result[1]) != std::end(commands)) {
    commands.at(result[1])(FunctionCallExpression::splitParameters(result[3]), local());
  } else {
    fork_exec(type, content);
  }
}

auto CustomCommandAction::test(const std::vector<std::string> & args, const Scope &) -> int
{
  std::cout << "test" << std::endl;

  for (auto iter = std::cbegin(args); iter != std::cend(args); ++iter) {
    std::cout << "  args[" << std::distance(std::cbegin(args), iter) << "] = " << *iter << "\n";
  }

  return args.size();
}

}  // namespace syntax
}  // namespace openscenario_interpreter
