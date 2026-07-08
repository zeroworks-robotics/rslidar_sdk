import os
import sys
import yaml
import tempfile
import subprocess
from getpass import getpass
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def install_cyclone_dds():
    result = subprocess.run(['dpkg', '-s', 'ros-humble-rmw-cyclonedds-cpp'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        print("DDS is already installed.")
    else:
        print("DDS not installed. Installing now...")
        password = getpass('Enter your sudo password to install DDS: ')
        try:
            subprocess.run(['sudo', '-S', 'apt-get', 'update'], input=password.encode(), check=True)
            subprocess.run(['sudo', '-S', 'apt-get', 'install', '-y', 'ros-humble-rmw-cyclonedds-cpp'], input=password.encode(), check=True)
            print("DDS installed successfully.")
        except subprocess.CalledProcessError as e:
            print(f"Failed to install DDS: {str(e)}")
            sys.exit(1)


def launch_setup(context, *args, **kwargs):
    pkg_dir = get_package_share_directory('rslidar_sdk')
    default_config_path = os.path.join(pkg_dir, 'config', 'config.yaml')

    lidar_frame_id   = LaunchConfiguration('lidar_frame_id').perform(context)
    imu_frame_id     = LaunchConfiguration('imu_frame_id').perform(context)
    device_address   = LaunchConfiguration('device_address').perform(context)
    device_port      = LaunchConfiguration('device_port').perform(context)
    use_lidar_clock  = LaunchConfiguration('use_lidar_clock').perform(context)

    with open(default_config_path, 'r') as f:
        config = yaml.safe_load(f)

    # lidar (data) 설정 반영
    for lidar in config.get('lidar', []):
        ros_cfg = lidar.get('ros', {})
        if lidar_frame_id:
            ros_cfg['ros_frame_id'] = lidar_frame_id
        if imu_frame_id:
            ros_cfg['ros_imu_frame_id'] = imu_frame_id
        lidar['ros'] = ros_cfg

        # use_lidar_clock 오버라이드: 빈 문자열이면 config.yaml 값을 그대로 사용,
        # 값이 주어지면(true/false) 그 값으로 덮어씀
        if use_lidar_clock.strip():
            driver_cfg = lidar.get('driver', {})
            driver_cfg['use_lidar_clock'] = use_lidar_clock.strip().lower() in ('true', '1', 'yes', 'on')
            lidar['driver'] = driver_cfg

    # ctrl 설정 반영
    ctrl_cfg = config.get('ctrl', {})
    if device_address:
        ctrl_cfg['device_address'] = device_address
    if device_port:
        ctrl_cfg['device_port'] = int(device_port)
    config['ctrl'] = ctrl_cfg

    tmp = tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False)
    yaml.dump(config, tmp)
    tmp.close()

    return [
        Node(
            namespace='rslidar_sdk',
            package='rslidar_sdk',
            executable='rslidar_sdk_node',
            name='rslidar_sdk_node',
            output='screen',
            parameters=[{'config_path': tmp.name}],
        ),
        Node(
            namespace='rslidar_sdk',
            package='rslidar_sdk',
            executable='rslidar_ctrl_node',
            name='rslidar_ctrl_node',
            output='screen',
            parameters=[{'config_path': tmp.name}],
        ),
    ]


def generate_launch_description():
    if os.getenv('ROS_DISTRO') == 'humble':
        print("Detected ROS 2 Humble. Checking DDS...")
        install_cyclone_dds()
        os.environ['RMW_IMPLEMENTATION'] = 'rmw_cyclonedds_cpp'
        print(f"Environment Variable Set: RMW_IMPLEMENTATION={os.environ.get('RMW_IMPLEMENTATION')}")

    return LaunchDescription([
        DeclareLaunchArgument(
            'lidar_frame_id',
            default_value='rslidar',
            description='LiDAR frame ID (point cloud header, parent frame of /tf_static)',
        ),
        DeclareLaunchArgument(
            'imu_frame_id',
            default_value='rslidar_imu',
            description='IMU frame ID (child frame of /tf_static)',
        ),
        DeclareLaunchArgument(
            'device_address',
            default_value='192.168.1.200',
            description='LiDAR IP address (used for ctrl TCP connection)',
        ),
        DeclareLaunchArgument(
            'device_port',
            default_value='6699',
            description='LiDAR control port',
        ),
        DeclareLaunchArgument(
            'use_lidar_clock',
            default_value='',
            description="타임스탬프 시계 선택. ''=config.yaml 값 유지, "
                        "'true'=라이다 내부 시계, 'false'=PC 시스템 시계",
        ),
        OpaqueFunction(function=launch_setup),
    ])
