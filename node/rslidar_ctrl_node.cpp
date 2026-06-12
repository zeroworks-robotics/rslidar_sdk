#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <std_srvs/srv/set_bool.hpp>
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <yaml-cpp/yaml.h>
#include <signal.h>

#include "lidar_ctrl_driver.hpp"
#include "ctrl_driver_param.hpp"

using namespace robosense::lidar;

class RslidarCtrlNode : public rclcpp::Node
{
public:
  explicit RslidarCtrlNode(const std::string& config_path)
    : Node("rslidar_ctrl_node")
  {
    YAML::Node config;
    try
    {
      config = YAML::LoadFile(config_path);
    }
    catch (...)
    {
      RCLCPP_ERROR(get_logger(), "Failed to load config: %s", config_path.c_str());
      throw;
    }

    auto ctrl_cfg        = config["ctrl"];
    device_address_      = ctrl_cfg["device_address"].as<std::string>("192.168.1.200");
    device_port_         = ctrl_cfg["device_port"].as<uint16_t>(6699);
    std::string type_str = ctrl_cfg["lidar_type"].as<std::string>("CRS_AIRY");
    lidar_type_          = strToCtrlLidarType(type_str);

    reboot_srv_ = create_service<std_srvs::srv::Trigger>(
      "rslidar/reboot",
      std::bind(&RslidarCtrlNode::handleReboot, this,
                std::placeholders::_1, std::placeholders::_2));

    power_srv_ = create_service<std_srvs::srv::SetBool>(
      "rslidar/set_power",
      std::bind(&RslidarCtrlNode::handleSetPower, this,
                std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(get_logger(), "rslidar_ctrl_node ready. LiDAR: %s:%d",
                device_address_.c_str(), device_port_);
    RCLCPP_INFO(get_logger(), "  /rslidar/reboot     (std_srvs/Trigger)  -- 완전 재부팅");
    RCLCPP_INFO(get_logger(), "  /rslidar/set_power  (std_srvs/SetBool)  -- true=ON / false=OFF");
  }

private:
  bool connectCtrl(LidarCtrlDriver& ctrl)
  {
    RSCtrlDriverParam param;
    param.device_address = device_address_;
    param.device_port    = device_port_;
    param.lidar_type     = lidar_type_;
    return ctrl.init(param);
  }

  void handleReboot(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> /*req*/,
    std::shared_ptr<std_srvs::srv::Trigger::Response> res)
  {
    RCLCPP_INFO(get_logger(), "Reboot requested");
    LidarCtrlDriver ctrl;
    if (!connectCtrl(ctrl))
    {
      res->success = false;
      res->message = "Failed to connect to LiDAR (" + device_address_ + ")";
      RCLCPP_ERROR(get_logger(), "%s", res->message.c_str());
      return;
    }
    bool ok = ctrl.rebootLidar();
    ctrl.uninit();
    res->success = ok;
    res->message = ok ? "Reboot command sent" : "rebootLidar() failed";
    RCLCPP_INFO(get_logger(), "%s", res->message.c_str());
  }

  void handleSetPower(
    const std::shared_ptr<std_srvs::srv::SetBool::Request> req,
    std::shared_ptr<std_srvs::srv::SetBool::Response> res)
  {
    const char* label = req->data ? "ON" : "OFF";
    RCLCPP_INFO(get_logger(), "set_power requested: %s", label);
    LidarCtrlDriver ctrl;
    if (!connectCtrl(ctrl))
    {
      res->success = false;
      res->message = "Failed to connect to LiDAR (" + device_address_ + ")";
      RCLCPP_ERROR(get_logger(), "%s", res->message.c_str());
      return;
    }
    bool ok = ctrl.setMode(req->data ? 1 : 0);
    ctrl.uninit();
    res->success = ok;
    res->message = ok
      ? (req->data ? "LiDAR powered ON" : "LiDAR powered OFF")
      : "setMode() failed";
    RCLCPP_INFO(get_logger(), "%s", res->message.c_str());
  }

  std::string   device_address_;
  uint16_t      device_port_;
  CtrlLidarType lidar_type_;

  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr reboot_srv_;
  rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr power_srv_;
};

static void sigHandler(int) { rclcpp::shutdown(); }

int main(int argc, char** argv)
{
  signal(SIGINT, sigHandler);
  rclcpp::init(argc, argv);

  // config_path 파라미터 수신 (rslidar_sdk_node와 동일한 방식)
  std::string config_path =
    ament_index_cpp::get_package_share_directory("rslidar_sdk") + "/config/config.yaml";

  auto param_nd = rclcpp::Node::make_shared("ctrl_param_handle");
  std::string path = param_nd->declare_parameter<std::string>("config_path", "");
  if (!path.empty())
    config_path = path;

  RCLCPP_INFO(param_nd->get_logger(), "ctrl config: %s", config_path.c_str());

  auto node = std::make_shared<RslidarCtrlNode>(config_path);
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
