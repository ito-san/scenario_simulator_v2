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

#ifndef CONCEALER__AUTOWARE_ERROR_HPP_
#define CONCEALER__AUTOWARE_ERROR_HPP_

#include <stdexcept>

namespace concealer
{
struct AutowareError : public std::runtime_error
{
  using std::runtime_error::runtime_error;

  virtual ~AutowareError() = default;

  virtual void raise() const { throw *this; }
};
}  // namespace concealer

#endif  // CONCEALER__AUTOWARE_ERROR_HPP_