# Standalone de-skew relay for /rslidar_points, includable from a bringup.
#
# It sits between the driver and every consumer: subscribe the driver's raw topic, give each
# point the transform belonging to its own moment in the 100 ms sweep, and republish in the
# same frame at the same stamp. See node/rslidar_deskew_node.cpp for the measurements that
# justify it.
#
#   ros2 launch rslidar_sdk deskew.launch.py
#   ros2 launch rslidar_sdk deskew.launch.py fixed_frame:=odom sweep_duration:=0.1
#
# TO ACTUALLY TAKE EFFECT, the consumer has to be pointed at the output. For STVL that is two
# lines in cona_stvl_costmap.yaml -- the lidar_mark and lidar_clear source topics:
#
#   topic: /rslidar_points            ->   topic: /rslidar_points_deskewed
#
# Leaving them on the raw topic costs one extra republish and changes nothing else, so the
# relay is safe to run before that switch is made.

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    args = [
        DeclareLaunchArgument(
            'input_topic', default_value='/rslidar_points',
            description="Driver's raw cloud. Must stay organised (height = sweep rows)."),
        DeclareLaunchArgument(
            'output_topic', default_value='/rslidar_points_deskewed',
            description='De-skewed cloud, same frame_id and stamp as the input.'),
        DeclareLaunchArgument(
            # odom, not map: it only has to be continuous across the 100 ms of one sweep, and
            # a localiser correction landing mid-sweep would be compensated into the points
            # as though the sensor had moved.
            'fixed_frame', default_value='odom',
            description='Frame the sweep motion is measured against.'),
        DeclareLaunchArgument(
            'sweep_duration', default_value='0.1',
            description='Seconds for one full sweep. RSAIRY at 10 Hz = 0.1. Ignored when the '
                        'cloud carries per-point timestamps (POINT_TYPE=XYZIRT).'),
        DeclareLaunchArgument(
            'stamp_at', default_value='first',
            description="Where header.stamp sits in the sweep. 'first' matches the driver's "
                        "config.yaml ts_first_point: true."),
        DeclareLaunchArgument(
            'tf_timeout', default_value='0.05',
            description='Seconds to wait for TF to reach the end of the sweep before giving '
                        'up and republishing unchanged.'),
        DeclareLaunchArgument(
            'rows_per_knot', default_value='10',
            description='TF lookups are taken every N rows and interpolated between. 1 = per '
                        'row. Rotation within one row is 111 us, so 10 is exact to well under '
                        'a voxel and costs a tenth of the lookups.'),
    ]

    def f(name, t=float):
        return ParameterValue(LaunchConfiguration(name), value_type=t)

    return LaunchDescription(args + [
        Node(
            package='rslidar_sdk', executable='rslidar_deskew_node',
            name='rslidar_deskew', output='screen',
            parameters=[{
                'input_topic': ParameterValue(LaunchConfiguration('input_topic'), value_type=str),
                'output_topic': ParameterValue(LaunchConfiguration('output_topic'), value_type=str),
                'fixed_frame': ParameterValue(LaunchConfiguration('fixed_frame'), value_type=str),
                'stamp_at': ParameterValue(LaunchConfiguration('stamp_at'), value_type=str),
                'sweep_duration': f('sweep_duration'),
                'tf_timeout': f('tf_timeout'),
                'rows_per_knot': f('rows_per_knot', int),
            }],
        ),
    ])
