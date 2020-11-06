// Copyright 2015-2020 Autoware Foundation. All rights reserved.
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

#ifndef OPEN_SCENARIO_INTERPRETER__OPEN_SCENARIO_INTERPRETER_HPP_
#define OPEN_SCENARIO_INTERPRETER__OPEN_SCENARIO_INTERPRETER_HPP_

#include <junit_exporter/junit_exporter.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <lifecycle_msgs/msg/transition.hpp>
#include <open_scenario_interpreter/syntax/open_scenario.hpp>
#include <open_scenario_interpreter/utility/verbose.hpp>
#include <open_scenario_interpreter/utility/visibility.h>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <string>
#include <utility>

namespace open_scenario_interpreter
{
class Interpreter
  : public rclcpp_lifecycle::LifecycleNode
{
  std::string expect;

  std::string osc_path;

  std::string map_path;

  std::string log_path;

  Element script;

  std::shared_ptr<rclcpp::TimerBase> timer;

  junit_exporter::JunitExporter exporter;

  int step_time_ms;

  const junit_exporter::TestResult ERROR = junit_exporter::TestResult::ERROR;
  const junit_exporter::TestResult FAILURE = junit_exporter::TestResult::FAILURE;
  const junit_exporter::TestResult SUCCESS = junit_exporter::TestResult::SUCCESS;

  decltype(auto) report(
    junit_exporter::TestResult result,
    const std::string & type,
    const std::string & what = "")
  {
    VERBOSE("  appending current test case result");
    exporter.addTestCase(
      script.as<OpenScenario>().scope.scenario.string(),  // XXX DIRTY HACK!!!
      "scenario_testing", 0, result, type, what);

    switch (result) {
      case junit_exporter::TestResult::ERROR:
        VERBOSE("\x1b[1;31mYield " << type.c_str() << " (" << what.c_str() << ")");
        break;

      case junit_exporter::TestResult::FAILURE:
        VERBOSE("\x1b[1;31mYield " << type.c_str() << " (" << what.c_str() << ")");
        break;

      case junit_exporter::TestResult::SUCCESS:
        VERBOSE("\x1b[32mYield " << type.c_str() << " (" << what.c_str() << ")");
        break;
    }
    VERBOSE("");

    exporter.write(log_path);

    script.reset();

    while (get_current_state().id() != lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE) {
      VERBOSE("  waiting for change current state " << get_current_state().id() << " to active");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    deactivate();
  }

  template<typename Thunk>
  void guard(Thunk && thunk) try
  {
    return thunk();
  } catch (const int command) {
    switch (command) {
      case EXIT_SUCCESS:
        if (expect == "success") {
          report(SUCCESS, "intended-success");
        } else {
          report(FAILURE, "unintended-failure", "expected " + expect);
        }
        break;

      case EXIT_FAILURE:
        if (expect == "failure") {
          report(SUCCESS, "intended-failure");
        } else {
          report(FAILURE, "unintended-success", "expected " + expect);
        }
        break;

      default:
        break;
    }
  } catch (const open_scenario_interpreter::ImplementationFault & error) {
    VERBOSE("  caught implementation-fault");
    if (expect == "error") {
      report(SUCCESS, "intended-error");
    } else {
      report(ERROR, "implementation-fault", error.what());
    }
  } catch (const std::exception & error) {
    VERBOSE(" caught standard exception");
    if (expect == "error") {
      report(SUCCESS, "intended-error");
    } else {
      report(ERROR, "unexpected-standard-exception", error.what());
    }
  } catch (...) {
    VERBOSE(" caught unknown exception");
    report(ERROR, "unexpected-unknown-exception");
  }

public:
  OPEN_SCENARIO_INTERPRETER_PUBLIC
  explicit Interpreter(const rclcpp::NodeOptions &);

  using Result = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

  Result on_configure(const rclcpp_lifecycle::State &) override;

  Result on_activate(const rclcpp_lifecycle::State &) override;

  Result on_deactivate(const rclcpp_lifecycle::State &) override;

  Result on_cleanup(const rclcpp_lifecycle::State &) override;

  Result on_shutdown(const rclcpp_lifecycle::State &) override;

  Result on_error(const rclcpp_lifecycle::State &) override;
};
}  // namespace open_scenario_interpreter

#endif  // OPEN_SCENARIO_INTERPRETER__OPEN_SCENARIO_INTERPRETER_HPP_