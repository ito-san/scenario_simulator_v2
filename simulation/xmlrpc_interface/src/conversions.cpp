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

#include <xmlrpc_interface/conversions.hpp>

#include <string>
#include <vector>

namespace xmlrpc_interface
{
void toProto(const XmlRpc::XmlRpcValue & from, simulation_api_schema::InitializeResponse & to)
{
  to = simulation_api_schema::InitializeResponse();
  to.mutable_result()->set_success(xmlrpc_interface::getXmlValue<bool>(from, key::success));
  to.mutable_result()->set_description(
    xmlrpc_interface::getXmlValue<std::string>(
      from,
      key::description));
}

void fromProto(const simulation_api_schema::InitializeResponse & from, XmlRpc::XmlRpcValue & to)
{
  to = XmlRpc::XmlRpcValue();
  to[key::success] = from.result().success();
  to[key::description] = from.result().description();
}

void toProto(const XmlRpc::XmlRpcValue & from, simulation_api_schema::InitializeRequest & to)
{
  to = simulation_api_schema::InitializeRequest();
  to.set_realtime_factor(xmlrpc_interface::getXmlValue<double>(from, key::realtime_factor));
  to.set_step_time(xmlrpc_interface::getXmlValue<double>(from, key::step_time));
}

void fromProto(const simulation_api_schema::InitializeRequest & from, XmlRpc::XmlRpcValue & to)
{
  to = XmlRpc::XmlRpcValue();
  to[key::realtime_factor] = from.realtime_factor();
  to[key::step_time] = from.step_time();
}

void toProto(const XmlRpc::XmlRpcValue & from, simulation_api_schema::UpdateFrameRequest & to)
{
  to = simulation_api_schema::UpdateFrameRequest();
  to.set_current_time(from[key::current_time]);
}

void fromProto(const simulation_api_schema::UpdateFrameRequest & from, XmlRpc::XmlRpcValue & to)
{
  to = XmlRpc::XmlRpcValue();
  to[key::current_time] = from.current_time();
}

void toProto(const XmlRpc::XmlRpcValue & from, simulation_api_schema::UpdateFrameResponse & to)
{
  to = simulation_api_schema::UpdateFrameResponse();
  to.mutable_result()->set_success(from[key::success]);
  to.mutable_result()->set_description(from[key::description]);
}

void fromProto(const simulation_api_schema::UpdateFrameResponse & from, XmlRpc::XmlRpcValue & to)
{
  to = XmlRpc::XmlRpcValue();
  to[key::success] = from.result().success();
  to[key::description] = from.result().description();
}

void toProto(const geometry_msgs::msg::Point & p, geometry_msgs::Point & proto)
{
  proto.set_x(p.x);
  proto.set_y(p.y);
  proto.set_z(p.z);
}

void toProto(const geometry_msgs::msg::Quaternion & q, geometry_msgs::Quaternion & proto)
{
  proto.set_x(q.x);
  proto.set_y(q.y);
  proto.set_z(q.z);
  proto.set_w(q.w);
}

void toProto(const geometry_msgs::msg::Pose & p, geometry_msgs::Pose & proto)
{
  toProto(p.position, *proto.mutable_position());
  toProto(p.orientation, *proto.mutable_orientation());
}
}  // namespace xmlrpc_interface
