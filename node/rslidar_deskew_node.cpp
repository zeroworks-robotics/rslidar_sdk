// Motion-compensate (de-skew) an RSAIRY point cloud before anything consumes it.
//
// THE PROBLEM
//
// The RSAIRY sweeps 360 deg in 100 ms and the driver stamps the finished cloud with the
// time of its FIRST point (config.yaml: ts_first_point: true). Every consumer then applies
// ONE transform, taken at that stamp, to all 86,400 points -- so everything captured later
// in the sweep is placed as if the robot had not moved since the sweep began. Measured on
// the robot (bag 20260903_1810, 90 s, 896 clouds):
//
//   at 1.2 m/s straight   a 0 -> 12 cm displacement gradient across one cloud
//   at 2.06 rad/s in place a wall 5 m away is displaced by up to 1 m
//
// In the costmap that renders as walls drawn several voxels thick, and it costs 4.7% of the
// cells the chassis could stand in while turning. Correcting it offline recovered 100% of
// the excess in a single cloud and 102% of the lost standable cells.
//
// THE FIX
//
// Give every point the transform belonging to ITS OWN capture time, then express the result
// back in the sensor frame at the cloud's header stamp:
//
//   p_out = T(sensor@t_ref <- fixed) * T(fixed <- sensor@t_i) * p_i
//
// Output therefore keeps the input's frame_id, stamp, field layout and organised shape, so
// nothing downstream needs to change -- STVL and the localiser see the same contract they
// see now, with the points in the right places.
//
// WHERE t_i COMES FROM
//
// Preferred: the cloud's own per-point `timestamp` field, which the driver fills when built
// with POINT_TYPE=XYZIRT. That is robust to any change of scan rate, phase or start angle.
//
// Fallback (what the deployed XYZI build needs): reconstruct it from the row index. The
// cloud is organised height x width with azimuth decreasing monotonically down the rows, so
// row index IS position within the sweep:
//
//   t_row = header.stamp + (row / height) * sweep_duration
//
// Verified against the same bag: with this reconstruction a single cloud taken at 2 rad/s
// de-skews to layer index 1.70 against a stationary 1.70, and the result is flat to +-10%
// changes in sweep_duration and +-10 ms of phase, so there is nothing to tune.
//
// ON FAILURE
//
// If TF cannot cover the sweep, the cloud is republished UNCHANGED rather than dropped.
// A dropped lidar cloud starves the costmap past voxel_decay -- measured elsewhere as the
// grid going completely blank -- which is far worse than a skewed one.

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Transform.h"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

namespace
{
tf2::Transform toTf2(const geometry_msgs::msg::TransformStamped & m)
{
  return tf2::Transform(
    tf2::Quaternion(
      m.transform.rotation.x, m.transform.rotation.y,
      m.transform.rotation.z, m.transform.rotation.w),
    tf2::Vector3(
      m.transform.translation.x, m.transform.translation.y,
      m.transform.translation.z));
}
}  // namespace

class DeskewNode : public rclcpp::Node
{
public:
  DeskewNode()
  : rclcpp::Node("rslidar_deskew")
  {
    input_topic_ = declare_parameter<std::string>("input_topic", "/rslidar_points");
    output_topic_ = declare_parameter<std::string>("output_topic", "/rslidar_points_deskewed");
    // The frame the motion is measured against. odom, not map: it has to be continuous over
    // the 100 ms of one sweep, and a localiser correction landing mid-sweep would be read as
    // sensor motion and compensated into the points.
    fixed_frame_ = declare_parameter<std::string>("fixed_frame", "odom");
    sweep_duration_ = declare_parameter<double>("sweep_duration", 0.1);
    // "first" matches the driver's ts_first_point: true -- the stamp is the first point, so
    // the sweep runs [stamp, stamp + sweep_duration]. "last" runs [stamp - sweep, stamp].
    stamp_at_ = declare_parameter<std::string>("stamp_at", "first");
    // How long to wait for TF to reach the end of the sweep. The cloud arrives ~113 ms after
    // its stamp and odom->base_link runs at 50 Hz, so this is normally already satisfied.
    tf_timeout_ = declare_parameter<double>("tf_timeout", 0.05);
    // One lookup per row would be 900 per cloud. Rotation within a single row is negligible
    // (111 us), so interpolating between knots costs nothing measurable and cuts the lookups
    // by this factor. 0 or 1 means per-row.
    rows_per_knot_ = std::max<int>(1, declare_parameter<int>("rows_per_knot", 10));

    if (stamp_at_ != "first" && stamp_at_ != "last") {
      RCLCPP_WARN(
        get_logger(), "stamp_at '%s' is neither first nor last; using first",
        stamp_at_.c_str());
      stamp_at_ = "first";
    }

    buffer_ = std::make_shared<tf2_ros::Buffer>(get_clock());
    // spin_thread = true, and the matching setUsingDedicatedThread. The callback below waits
    // for TF to reach the END of the sweep, which is ~100 ms past the cloud's stamp. With the
    // listener on this node's own executor that wait can never be satisfied -- the thread
    // that would deliver /tf is the one blocked waiting for it -- and tf2 says so outright:
    //   "Do not call canTransform or lookupTransform with a timeout unless you are using
    //    another thread for populating data. Without a dedicated thread it will always
    //    timeout."
    // Measured before this was fixed: 302 clouds, 0 de-skewed, 302 passed through.
    listener_ = std::make_shared<tf2_ros::TransformListener>(*buffer_, this, true);
    buffer_->setUsingDedicatedThread(true);

    pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
      output_topic_, rclcpp::SensorDataQoS());
    sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
      input_topic_, rclcpp::SensorDataQoS(),
      std::bind(&DeskewNode::onCloud, this, std::placeholders::_1));

    RCLCPP_INFO(
      get_logger(), "de-skew %s -> %s | fixed_frame %s | sweep %.0f ms stamped at %s",
      input_topic_.c_str(), output_topic_.c_str(), fixed_frame_.c_str(),
      sweep_duration_ * 1e3, stamp_at_.c_str());
  }

private:
  void onCloud(const sensor_msgs::msg::PointCloud2::ConstSharedPtr in)
  {
    ++seen_;
    const bool has_ts = hasField(*in, "timestamp");
    if (once_) {
      once_ = false;
      RCLCPP_INFO(
        get_logger(),
        "first cloud: %ux%u, point_step %u, per-point timestamp field: %s",
        in->width, in->height, in->point_step,
        has_ts ? "yes (exact)" : "no (reconstructing from row index)");
      if (!has_ts && in->height <= 1) {
        RCLCPP_ERROR(
          get_logger(),
          "cloud is not organised (height=%u) and carries no timestamp field, so per-point "
          "times cannot be recovered. Passing everything through unchanged. Fix: build "
          "rslidar_sdk with POINT_TYPE=XYZIRT, or keep send_by_rows so the cloud stays "
          "organised.", in->height);
      }
    }

    if (!has_ts && in->height <= 1) { pub_->publish(*in); return; }

    // Times bounding the sweep, and the reference the output is expressed in.
    const rclcpp::Time t_ref(in->header.stamp, RCL_ROS_TIME);
    const double sweep = sweep_duration_;
    const rclcpp::Time t_lo = (stamp_at_ == "first") ? t_ref : t_ref - rclcpp::Duration::from_seconds(sweep);
    const rclcpp::Time t_hi = (stamp_at_ == "first") ? t_ref + rclcpp::Duration::from_seconds(sweep) : t_ref;

    // Wait once, for the far end. If that is available every earlier knot is too.
    std::string err;
    if (!buffer_->canTransform(
        fixed_frame_, in->header.frame_id, t_hi,
        rclcpp::Duration::from_seconds(tf_timeout_), &err))
    {
      passthrough(in, err);
      return;
    }

    tf2::Transform ref_inv;
    try {
      ref_inv = toTf2(buffer_->lookupTransform(fixed_frame_, in->header.frame_id, t_ref)).inverse();
    } catch (const tf2::TransformException & ex) {
      passthrough(in, ex.what());
      return;
    }

    const uint32_t H = in->height;
    const uint32_t W_ = in->width;
    const int step = rows_per_knot_;
    const int n_knots = static_cast<int>((H + step - 1) / step) + 1;

    // Time of each knot. The driver's own per-point timestamps are preferred when the cloud
    // carries them (POINT_TYPE=XYZIRT): they survive any change of scan rate, phase or start
    // angle, which the row-index reconstruction below does not. A row with no usable stamp
    // (all returns invalid) falls back to the reconstruction for that knot alone.
    std::vector<rclcpp::Time> knot_time;
    knot_time.reserve(n_knots);
    {
      std::unique_ptr<sensor_msgs::PointCloud2ConstIterator<double>> its;
      if (has_ts) {
        its = std::make_unique<sensor_msgs::PointCloud2ConstIterator<double>>(*in, "timestamp");
      }
      const double ref_s = t_ref.seconds();
      for (int k = 0; k < n_knots; ++k) {
        const double frac = std::min(1.0, static_cast<double>(k) * step / static_cast<double>(H));
        rclcpp::Time tk = t_lo + rclcpp::Duration::from_seconds(frac * sweep);
        if (has_ts) {
          const uint32_t row = std::min<uint32_t>(H - 1, static_cast<uint32_t>(k) * step);
          for (uint32_t c = 0; c < W_; ++c) {
            const double ts = *(*its + (row * W_ + c));
            // Reject the obvious rubbish: unset stamps are 0 and a stamp further than a
            // second from the header cannot belong to this sweep.
            if (std::isfinite(ts) && std::fabs(ts - ref_s) < 1.0) {
              tk = rclcpp::Time(static_cast<int64_t>(ts * 1e9), t_ref.get_clock_type());
              break;
            }
          }
        }
        knot_time.push_back(tk);
      }
    }

    // sensor@t_ref <- sensor@t_knot, at each knot along the sweep
    std::vector<tf2::Transform> knots;
    knots.reserve(n_knots);
    try {
      for (int k = 0; k < n_knots; ++k) {
        knots.push_back(
          ref_inv *
          toTf2(buffer_->lookupTransform(fixed_frame_, in->header.frame_id, knot_time[k])));
      }
    } catch (const tf2::TransformException & ex) {
      passthrough(in, ex.what());
      return;
    }

    auto out = std::make_shared<sensor_msgs::msg::PointCloud2>(*in);
    sensor_msgs::PointCloud2Iterator<float> ix(*out, "x"), iy(*out, "y"), iz(*out, "z");

    for (uint32_t r = 0; r < H; ++r) {
      // Interpolate between the two knots bracketing this row. The rotation across one knot
      // spacing is at most a few tenths of a degree, so lerp on the translation and slerp on
      // the rotation is exact to well under a voxel.
      const double kf = static_cast<double>(r) / step;
      const int k0 = static_cast<int>(kf);
      const int k1 = std::min(k0 + 1, n_knots - 1);
      const double a = kf - k0;
      const tf2::Transform & A = knots[k0];
      const tf2::Transform & B = knots[k1];
      const tf2::Transform M(
        (a <= 0.0) ? A.getRotation() : A.getRotation().slerp(B.getRotation(), a),
        A.getOrigin() * (1.0 - a) + B.getOrigin() * a);

      for (uint32_t c = 0; c < W_; ++c, ++ix, ++iy, ++iz) {
        const float x = *ix, y = *iy, z = *iz;
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) { continue; }
        const tf2::Vector3 p = M * tf2::Vector3(x, y, z);
        *ix = static_cast<float>(p.x());
        *iy = static_cast<float>(p.y());
        *iz = static_cast<float>(p.z());
      }
    }

    ++deskewed_;
    pub_->publish(*out);
    report();
  }

  void passthrough(const sensor_msgs::msg::PointCloud2::ConstSharedPtr & in, const std::string & why)
  {
    ++passed_;
    // Throttled, not silent: a costmap quietly built from skewed clouds is the bug this node
    // exists to remove, so it must be visible when the node is not actually doing its job.
    RCLCPP_WARN_THROTTLE(
      get_logger(), *get_clock(), 5000,
      "TF does not cover the sweep, republishing unchanged (%s <- %s): %s",
      fixed_frame_.c_str(), in->header.frame_id.c_str(), why.c_str());
    pub_->publish(*in);
    report();
  }

  void report()
  {
    const auto now = get_clock()->now();
    if (last_report_.nanoseconds() == 0) { last_report_ = now; return; }
    if ((now - last_report_).seconds() < 30.0) { return; }
    last_report_ = now;
    RCLCPP_INFO(
      get_logger(), "%zu clouds: %zu de-skewed, %zu passed through unchanged",
      seen_, deskewed_, passed_);
  }

  static bool hasField(const sensor_msgs::msg::PointCloud2 & m, const std::string & name)
  {
    return std::any_of(
      m.fields.begin(), m.fields.end(),
      [&](const sensor_msgs::msg::PointField & f) { return f.name == name; });
  }

  std::string input_topic_, output_topic_, fixed_frame_, stamp_at_;
  double sweep_duration_{0.1}, tf_timeout_{0.05};
  int rows_per_knot_{10};
  bool once_{true};
  size_t seen_{0}, deskewed_{0}, passed_{0};
  rclcpp::Time last_report_{0, 0, RCL_ROS_TIME};
  std::shared_ptr<tf2_ros::Buffer> buffer_;
  std::shared_ptr<tf2_ros::TransformListener> listener_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DeskewNode>());
  rclcpp::shutdown();
  return 0;
}
