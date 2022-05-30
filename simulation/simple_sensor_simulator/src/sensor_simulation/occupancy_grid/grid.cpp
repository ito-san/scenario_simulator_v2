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

#include <quaternion_operation/quaternion_operation.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <rclcpp/rclcpp.hpp>
#include <simple_sensor_simulator/sensor_simulation/occupancy_grid/grid.hpp>

namespace simple_sensor_simulator
{
Grid::Grid(const geometry_msgs::msg::Pose & origin, double resolution, size_t height, size_t width)
: resolution(resolution), height(height), width(width), origin(origin), grid_cells_(getAllCells())
{
}

geometry_msgs::msg::Point Grid::transformToGrid(const geometry_msgs::msg::Point & world_point) const
{
  auto mat =
    quaternion_operation::getRotationMatrix(quaternion_operation::conjugate(origin.orientation));
  Eigen::VectorXd p(3);
  p(0) = world_point.x;
  p(1) = world_point.y;
  p(2) = world_point.z;
  p = mat * p;
  p(0) = p(0) - origin.position.x;
  p(1) = p(1) - origin.position.y;
  p(2) = p(2) - origin.position.z;
  geometry_msgs::msg::Point ret;
  ret.x = p(0);
  ret.y = p(1);
  ret.z = p(2);
  return ret;
}

LineSegment Grid::transformToGrid(const LineSegment & line) const
{
  return LineSegment(transformToGrid(line.start_point), transformToGrid(line.end_point));
}

geometry_msgs::msg::Point Grid::transformToPixel(const geometry_msgs::msg::Point & grid_point) const
{
  geometry_msgs::msg::Point p;
  p.x = (grid_point.x + height * resolution * 0.5) / resolution;
  p.y = (grid_point.y + width * resolution * 0.5) / resolution;
  p.z = 0;
  return p;
}

LineSegment Grid::transformToPixel(const LineSegment & line) const
{
  return LineSegment(transformToPixel(line.start_point), transformToPixel(line.end_point));
}

std::vector<GridCell> Grid::getAllCells() const
{
  std::vector<GridCell> ret;
  for (size_t x_index = 0; x_index < height; x_index++) {
    for (size_t y_index = 0; y_index < width; y_index++) {
      geometry_msgs::msg::Pose cell_origin;
      cell_origin.position.x = origin.position.x + (x_index - 0.5 * height) * resolution;
      cell_origin.position.y = origin.position.y + (y_index - 0.5 * width) * resolution;
      cell_origin.position.z = origin.position.z;
      cell_origin.orientation = origin.orientation;
      ret.emplace_back(
        GridCell(cell_origin, resolution, width * y_index + x_index, y_index, x_index));
    }
  }
  return ret;
}

std::array<LineSegment, 4> Grid::getOutsideLineSegments() const
{
  geometry_msgs::msg::Point left_up;
  left_up.x = origin.position.x + resolution * height * 0.5;
  left_up.y = origin.position.y + resolution * height * 0.5;
  left_up.z = origin.position.z;
  geometry_msgs::msg::Point left_down;
  left_down.x = origin.position.x - resolution * height * 0.5;
  left_down.y = origin.position.y + resolution * height * 0.5;
  left_down.z = origin.position.z;
  geometry_msgs::msg::Point right_up;
  right_up.x = origin.position.x + resolution * height * 0.5;
  right_up.y = origin.position.y - resolution * height * 0.5;
  right_up.z = origin.position.z;
  geometry_msgs::msg::Point right_down;
  right_down.x = origin.position.x - resolution * height * 0.5;
  right_down.y = origin.position.y - resolution * height * 0.5;
  right_down.z = origin.position.z;
  return {
    LineSegment(left_up, left_down), LineSegment(left_down, right_down),
    LineSegment(right_down, right_up), LineSegment(right_up, left_up)};
}

std::vector<geometry_msgs::msg::Point> Grid::raycastToOutside(
  const std::unique_ptr<simple_sensor_simulator::primitives::Primitive> & primitive) const
{
  std::vector<geometry_msgs::msg::Point> ret;
  const auto outsides = getOutsideLineSegments();
  const auto points = primitive->get2DConvexHull();
  for (const auto & point : points) {
    geometry_msgs::msg::Vector3 vec;
    vec.x = point.x - origin.position.x;
    vec.y = point.y - origin.position.y;
    vec.z = point.z - origin.position.z;
    const auto ray =
      LineSegment(origin.position, vec, std::hypot(width * resolution, height * resolution) * 2);
    for (const auto & outside : outsides) {
      const auto hit = outside.getIntersection2D(ray);
      if (hit) {
        ret.emplace_back(hit.get());
      }
    }
  }
  return ret;
}

size_t Grid::getNextRowIndex(size_t row, size_t col) const { return width * col + (row + 1); }
size_t Grid::getNextColINdex(size_t row, size_t col) const { return width * (col + 1) + row; }
size_t Grid::getPreviousRowIndex(size_t row, size_t col) const { return width * col + (row - 1); }
size_t Grid::getPreviousColINdex(size_t row, size_t col) const { return width * (col - 1) + row; }

void Grid::fillIntersectionCell(const LineSegment & line_segment, int8_t data)
{
  const auto line_segment_pixel = transformToPixel(transformToGrid(line_segment));
  int start_x_pixel = std::ceil(line_segment_pixel.start_point.x);
  int start_y_pixel = std::ceil(line_segment_pixel.start_point.y);
  int end_x_pixel = std::ceil(line_segment_pixel.end_point.x);
  int end_y_pixel = std::ceil(line_segment_pixel.end_point.y);
  if (start_x_pixel == end_x_pixel) {
    fillByRow(start_x_pixel, data);
    return;
  }
  if (start_y_pixel == end_y_pixel) {
    fillByCol(start_y_pixel, data);
    return;
  }
  if (start_x_pixel < end_x_pixel) {
    for (int x_pixel = start_x_pixel; x_pixel <= end_x_pixel; x_pixel++) {
      int y_pixel = std::ceil(
        line_segment_pixel.getSlope() * static_cast<double>(x_pixel) +
        line_segment_pixel.getIntercept());
      fillByRowCol(x_pixel, y_pixel, data);
      fillByIndex(getNextRowIndex(x_pixel, y_pixel), data);
    }
    for (int y_pixel = end_y_pixel; y_pixel <= end_y_pixel; y_pixel++) {
      int x_pixel = std::ceil(
        (static_cast<double>(y_pixel) - line_segment_pixel.getIntercept()) /
        line_segment_pixel.getSlope());
      fillByRowCol(x_pixel, y_pixel, data);
      fillByIndex(getNextColINdex(x_pixel, y_pixel), data);
    }
  }
}

void Grid::addPrimitive(const std::unique_ptr<primitives::Primitive> & primitive)
{
  for (const auto & line : getLineSegments(primitive->get2DConvexHull(origin))) {
    fillIntersectionCell(line, 100);
  }
}

std::vector<int8_t> Grid::getData()
{
  std::vector<int8_t> data;
  for (const auto & cell : grid_cells_) {
    data.emplace_back(cell.getData());
  }
  return data;
}

void Grid::fillByIndex(size_t index, int8_t data)
{
  if (index < grid_cells_.size()) {
    grid_cells_[index].setData(data);
  }
}

void Grid::fillByRowCol(size_t row, size_t col, int8_t data)
{
  fillByIndex(width * col + row, data);
}

void Grid::fillByRow(size_t row, int8_t data)
{
  if (row < height) {
    for (size_t col = 0; col < width; col++) {
      fillByRowCol(row, col, data);
    }
  }
}

void Grid::fillByCol(size_t col, int8_t data)
{
  if (col < width) {
    for (size_t row = 0; row < height; row++) {
      fillByRowCol(row, col, data);
    }
  }
}
}  // namespace simple_sensor_simulator
