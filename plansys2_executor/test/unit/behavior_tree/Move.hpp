// Copyright 2019 Intelligent Robotics Lab
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

#ifndef UNIT__BEHAVIOR_TREE__MOVE_HPP_
#define UNIT__BEHAVIOR_TREE__MOVE_HPP_

#include <string>

#include "geometry_msgs/msg/pose2_d.hpp"
#include "test_msgs/action/fibonacci.hpp"

#include "plansys2_executor/BTActionNode.hpp"
#include "behaviortree_cpp_v3/behavior_tree.h"
#include "behaviortree_cpp_v3/bt_factory.h"

namespace plansys2_bt_tests
{

class Move : public plansys2::BtActionNode<test_msgs::action::Fibonacci>
{
public:
  explicit Move(
    const std::string & xml_tag_name,
    const BT::NodeConfiguration & config = {});

  void on_tick() override;
  void on_success() override;

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<geometry_msgs::msg::Pose2D>("goal"),
      BT::OutputPort<int>("goal_reached"),
    };
  }

private:
  int goal_reached_;
};

}  // namespace plansys2_bt_tests

#endif  // UNIT__BEHAVIOR_TREE__MOVE_HPP_
