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

#include <limits>
#include <string>
#include <traffic_simulator/color_utils/color_utils.hpp>
#include <traffic_simulator/entity/exception.hpp>
#include <traffic_simulator/traffic_lights/traffic_light.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

namespace traffic_simulator
{
TrafficLight::TrafficLight(
  std::int64_t id,
  const std::unordered_map<TrafficLightColor, geometry_msgs::msg::Point> & color_positions,
  const std::unordered_map<TrafficLightArrow, geometry_msgs::msg::Point> & arrow_positions)
: id(id), color_positions_(color_positions), arrow_positions_(arrow_positions)
{
  color_changed_ = true;
  color_phase_.setState(TrafficLightColor::NONE);
  arrow_changed_ = true;
  arrow_phase_.setState(TrafficLightArrow::NONE);
}

void TrafficLight::setPosition(
  const TrafficLightColor & color, const geometry_msgs::msg::Point & position)
{
  color_positions_.insert({color, position});
}

void TrafficLight::setColorPhase(
  const std::vector<std::pair<double, TrafficLightColor>> & phase, double time_offset)
{
  color_phase_.setPhase(phase, time_offset);
}

void TrafficLight::setArrowPhase(
  const std::vector<std::pair<double, TrafficLightArrow>> & phase, double time_offset)
{
  arrow_phase_.setPhase(phase, time_offset);
}

void TrafficLight::setColor(TrafficLightColor color) { color_phase_.setState(color); }

void TrafficLight::setArrow(TrafficLightArrow arrow) { arrow_phase_.setState(arrow); }

double TrafficLight::getColorPhaseLength() const { return color_phase_.getPhaseLength(); }

double TrafficLight::getArrowPhaseLength() const { return arrow_phase_.getPhaseLength(); }

TrafficLightArrow TrafficLight::getArrow() const { return arrow_phase_.getState(); }

TrafficLightColor TrafficLight::getColor() const { return color_phase_.getState(); }

bool TrafficLight::colorChanged() const { return color_changed_; }

bool TrafficLight::arrowChanged() const { return arrow_changed_; }

void TrafficLight::update(double step_time)
{
  const auto previous_arrow = getArrow();
  arrow_phase_.update(step_time);
  const auto arrow = getArrow();
  if (previous_arrow == arrow) {
    arrow_changed_ = false;
  } else {
    arrow_changed_ = true;
  }
  const auto previous_color = getColor();
  color_phase_.update(step_time);
  const auto color = getColor();
  if (previous_color == color) {
    color_changed_ = false;
  } else {
    color_changed_ = true;
  }
}

const geometry_msgs::msg::Point TrafficLight::getPosition(const TrafficLightColor & color)
{
  if (color_positions_.count(color) == 0) {
    throw traffic_simulator::SimulationRuntimeError("target color does not exists");
  }
  return color_positions_.at(color);
}

const geometry_msgs::msg::Point TrafficLight::getPosition(const TrafficLightArrow & arrow)
{
  if (arrow_positions_.count(arrow) == 0) {
    throw traffic_simulator::SimulationRuntimeError("target arrow does not exists");
  }
  return arrow_positions_.at(arrow);
}
}  // namespace traffic_simulator