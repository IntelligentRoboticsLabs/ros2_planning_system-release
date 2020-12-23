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

#include "plansys2_domain_expert/DomainExpertNode.hpp"

#include <string>
#include <memory>
#include <vector>

#include "lifecycle_msgs/msg/state.hpp"

std::vector<std::string> tokenize(const std::string & string, const std::string & delim)
{
  std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
  std::vector<std::string> tokens;

  while (lastPos != std::string::npos) {
    if (pos != lastPos) {
      tokens.push_back(string.substr(lastPos, pos - lastPos));
    }
    lastPos = pos;
    if (lastPos == std::string::npos || lastPos + 1 == string.length()) {
      break;
    }
    pos = string.find_first_of(delim, ++lastPos);
  }

  return tokens;
}

namespace plansys2
{

DomainExpertNode::DomainExpertNode()
: rclcpp_lifecycle::LifecycleNode("domain_expert")
{
  declare_parameter("model_file", "");

  get_types_service_ = create_service<plansys2_msgs::srv::GetDomainTypes>(
    "domain_expert/get_domain_types",
    std::bind(
      &DomainExpertNode::get_domain_types_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
  get_domain_actions_service_ = create_service<plansys2_msgs::srv::GetDomainActions>(
    "domain_expert/get_domain_actions",
    std::bind(
      &DomainExpertNode::get_domain_actions_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
  get_domain_action_details_service_ =
    create_service<plansys2_msgs::srv::GetDomainActionDetails>(
    "domain_expert/get_domain_action_details", std::bind(
      &DomainExpertNode::get_domain_action_details_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
  get_domain_predicates_service_ = create_service<plansys2_msgs::srv::GetDomainPredicates>(
    "domain_expert/get_domain_predicates", std::bind(
      &DomainExpertNode::get_domain_predicates_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
  get_domain_predicate_details_service_ =
    create_service<plansys2_msgs::srv::GetDomainPredicateDetails>(
    "domain_expert/get_domain_predicate_details", std::bind(
      &DomainExpertNode::get_domain_predicate_details_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
  get_domain_service_ = create_service<plansys2_msgs::srv::GetDomain>(
    "domain_expert/get_domain", std::bind(
      &DomainExpertNode::get_domain_service_callback,
      this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
}


using CallbackReturnT =
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

CallbackReturnT
DomainExpertNode::on_configure(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Configuring...", get_name());
  std::string model_file = get_parameter("model_file").get_value<std::string>();

  auto model_files = tokenize(model_file, ":");

  std::ifstream domain_ifs(model_files[0]);
  std::string domain_str((
      std::istreambuf_iterator<char>(domain_ifs)),
    std::istreambuf_iterator<char>());

  domain_expert_ = std::make_shared<DomainExpert>(domain_str);

  for (size_t i = 1; i < model_files.size(); i++) {
    std::ifstream domain_ifs(model_files[i]);
    std::string domain_str((
        std::istreambuf_iterator<char>(domain_ifs)),
      std::istreambuf_iterator<char>());
    domain_expert_->extendDomain(domain_str);
  }

  RCLCPP_INFO(get_logger(), "[%s] Configured", get_name());
  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
DomainExpertNode::on_activate(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Activating...", get_name());
  RCLCPP_INFO(get_logger(), "[%s] Activated", get_name());

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
DomainExpertNode::on_deactivate(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Deactivating...", get_name());
  RCLCPP_INFO(get_logger(), "[%s] Deactivated", get_name());

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
DomainExpertNode::on_cleanup(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Cleaning up...", get_name());
  RCLCPP_INFO(get_logger(), "[%s] Cleaned up", get_name());

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
DomainExpertNode::on_shutdown(const rclcpp_lifecycle::State & state)
{
  RCLCPP_INFO(get_logger(), "[%s] Shutting down...", get_name());
  RCLCPP_INFO(get_logger(), "[%s] Shutted down", get_name());

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
DomainExpertNode::on_error(const rclcpp_lifecycle::State & state)
{
  RCLCPP_ERROR(get_logger(), "[%s] Error transition", get_name());

  return CallbackReturnT::SUCCESS;
}

void
DomainExpertNode::get_domain_types_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainTypes::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainTypes::Response> response)
{
  if (domain_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = true;
    response->types = domain_expert_->getTypes();
  }
}

void
DomainExpertNode::get_domain_actions_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainActions::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainActions::Response> response)
{
  if (domain_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = true;
    for (const auto & action : domain_expert_->getActions()) {
      response->actions.push_back(action);
      response->type.push_back("action");
    }
    for (const auto & action : domain_expert_->getDurativeActions()) {
      response->actions.push_back(action);
      response->type.push_back("durative-action");
    }
  }
}

void
DomainExpertNode::get_domain_action_details_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainActionDetails::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainActionDetails::Response> response)
{
  if (domain_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";

    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    auto action = domain_expert_->getAction(request->action);
    auto durative_action = domain_expert_->getDurativeAction(request->action);

    if (action) {
      response->name = request->action;
      response->type = "action";

      for (const auto & param :  action.value().parameters) {
        response->param_names.push_back(param.name);
        response->param_types.push_back(param.type);
      }
      response->at_start_requirements = action.value().preconditions.toString();
      response->at_start_effects = action.value().effects.toString();

      response->success = true;
    } else if (durative_action) {
      response->name = request->action;
      response->type = "durative-action";

      for (const auto & param :  durative_action.value().parameters) {
        response->param_names.push_back(param.name);
        response->param_types.push_back(param.type);
      }

      response->at_start_requirements = durative_action.value().at_start_requirements.toString();
      response->over_all_requirements = durative_action.value().over_all_requirements.toString();
      response->at_end_requirements = durative_action.value().at_end_requirements.toString();
      response->at_start_effects = durative_action.value().at_start_effects.toString();
      response->at_end_effects = durative_action.value().at_end_effects.toString();

      response->success = true;
    } else {
      RCLCPP_WARN(get_logger(), "Requesting a non-existing action [%s]", request->action.c_str());
      response->success = false;
      response->error_info = "Action not found";
    }
  }
}

void
DomainExpertNode::get_domain_predicates_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainPredicates::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainPredicates::Response> response)
{
  if (domain_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = true;
    response->predicates = domain_expert_->getPredicates();
  }
}

void
DomainExpertNode::get_domain_predicate_details_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainPredicateDetails::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetDomainPredicateDetails::Response> response)
{
  if (domain_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    auto params = domain_expert_->getPredicate(request->predicate);
    if (params) {
      response->name = request->predicate;

      for (const auto & param :  params.value().parameters) {
        response->param_names.push_back(param.name);
        response->param_types.push_back(param.type);
      }

      response->success = true;
    } else {
      RCLCPP_WARN(
        get_logger(), "Requesting a non-existing predicate [%s]",
        request->predicate.c_str());
      response->success = false;
      response->error_info = "Predicate not found";
    }
  }
}

void
DomainExpertNode::get_domain_service_callback(
  const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<plansys2_msgs::srv::GetDomain::Request> request,
  const std::shared_ptr<plansys2_msgs::srv::GetDomain::Response> response)
{
  if (domain_expert_ == nullptr) {
    response->success = false;
    response->error_info = "Requesting service in non-active state";
    RCLCPP_WARN(get_logger(), "Requesting service in non-active state");
  } else {
    response->success = true;

    std::ostringstream stream;
    stream << domain_expert_->getDomain();
    response->domain = stream.str();
  }
}


}  // namespace plansys2
