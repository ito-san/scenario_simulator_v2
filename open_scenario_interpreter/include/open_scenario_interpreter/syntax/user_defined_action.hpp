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

#ifndef OPEN_SCENARIO_INTERPRETER__SYNTAX__USER_DEFINED_ACTION_HPP_
#define OPEN_SCENARIO_INTERPRETER__SYNTAX__USER_DEFINED_ACTION_HPP_

#include <open_scenario_interpreter/syntax/custom_command_action.hpp>

namespace open_scenario_interpreter
{
inline namespace syntax
{
/* ==== UserDefinedAction ====================================================
 *
 * <xsd:complexType name="UserDefinedAction">
 *   <xsd:sequence>
 *     <xsd:element name="CustomCommandAction" type="CustomCommandAction"/>
 *   </xsd:sequence>
 * </xsd:complexType>
 *
 * ======================================================================== */
struct UserDefinedAction
  : public Element
{
  template<typename Node, typename Scope>
  explicit UserDefinedAction(const Node & node, Scope & scope)
  {
    callWithElements(
      node, "CustomCommandAction", 1, 1, [&](auto && node)
      {
        return rebind<CustomCommandAction>(node, scope);
      });
  }
};
}
}  // namespace open_scenario_interpreter

#endif  // OPEN_SCENARIO_INTERPRETER__SYNTAX__USER_DEFINED_ACTION_HPP_