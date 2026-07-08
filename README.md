# rslidar_sdk

RoboSense RS-LiDAR-AIRY 전용 ROS2 패키지입니다.  
공식 [rslidar_sdk](https://github.com/RoboSense-LiDAR/rslidar_sdk) 기반에 아래 기능을 추가했습니다.

- LiDAR ↔ IMU extrinsic 보정값을 DIFOP 패킷에서 읽어 `/tf_static`으로 publish
- ROS2 서비스(`rslidar/reboot`, `rslidar/set_power`)를 통한 LiDAR 전원/재부팅 제어

---

## 1 패키지 구성

이 저장소 자체가 하나의 ROS2 패키지(`rslidar_sdk`)이며, ROS2 워크스페이스의 `src/` 아래에 클론해 사용합니다.

```
rslidar_sdk/              # 이 저장소 = LiDAR SDK 패키지
├── src/                  # 패키지 소스 (manager / msg / source / utility)
├── config/ node/ launch/ lib/ rviz/ doc/
└── thirdparty/
    ├── rs_driver/        # 드라이버 코어 (submodule: zeroworks-robotics/rs_driver)
    └── rslidar_msg/      # 메시지 정의 패키지 (submodule: zeroworks-robotics/rslidar_msg)
```

---

## 2 의존성

### 2.1 운영체제 / ROS2

| 항목 | 버전 |
|------|------|
| Ubuntu | 22.04 |
| ROS2 | Humble |

ROS2 Humble 설치: https://docs.ros.org/en/humble/Installation.html

### 2.2 시스템 라이브러리

```sh
sudo apt-get update
sudo apt-get install -y libyaml-cpp-dev libpcap-dev \
  libboost-dev libboost-system-dev libboost-date-time-dev
```

> `libboost-*` 는 `rslidar_ctrl_node`(LiDAR 제어)용입니다. [2.4](#24-lidar-제어-드라이버) 참고.

### 2.3 ROS2 패키지

```sh
sudo apt-get install -y \
  ros-humble-geometry-msgs \
  ros-humble-tf2-ros \
  ros-humble-std-srvs
```

### 2.4 LiDAR 제어 드라이버

`rslidar_ctrl_node`(전원/재부팅 서비스)의 제어 드라이버는 `lib/rs_ctrl_driver/src/`의
**소스를 직접 컴파일**합니다. 별도 바이너리 배치가 필요 없습니다.

- 의존성은 시스템 Boost 헤더뿐입니다 ([2.2](#22-시스템-라이브러리)에서 설치).
- 이전에는 사전 컴파일된 `librs_ctrl_driver.a` / `libboost_system.a`(x86-64 전용)를
  링크했으나, Jetson(aarch64) 등에서 아키텍처 불일치로 링크가 실패해 소스 빌드로 전환했습니다.
- `lib/rs_ctrl_driver/src/`에 소스가 없으면 CMake 경고 후 `rslidar_ctrl_node` 빌드를 건너뜁니다.

자세한 내용은 [`lib/rs_ctrl_driver/README.md`](lib/rs_ctrl_driver/README.md) 참고.

---

## 3 다운로드

워크스페이스의 `src/` 아래에 서브모듈까지 한 번에 클론합니다.

```sh
mkdir -p ~/ros2_ws/src && cd ~/ros2_ws/src
git clone --recursive <이_저장소_URL>
```

이미 클론한 경우 서브모듈을 별도로 초기화합니다.

```sh
git submodule update --init --recursive
```

> `rslidar_msg` 메시지는 `thirdparty/rslidar_msg` 서브모듈을 CMake `add_subdirectory`로
> **인라인 빌드**합니다. 별도 colcon 패키지나 심볼릭 링크 없이 아래 빌드만 하면 됩니다.

---

## 4 빌드

```sh
cd ~/ros2_ws
colcon build --symlink-install
source install/setup.bash
```

> `rslidar_ctrl_node`가 빌드되지 않는다면 [2.4 LiDAR 제어 드라이버](#24-lidar-제어-드라이버-선택) 항목을 확인하세요.

---

## 5 설정

### 5.1 config.yaml

`config/config.yaml`에서 기본값을 수정합니다.

주요 항목:

| 섹션 | 파라미터 | 설명 |
|------|---------|------|
| `ctrl` | `device_address` | LiDAR IP 주소 |
| `ctrl` | `device_port` | LiDAR 제어 포트 |
| `lidar[0].driver` | `msop_port` | 포인트 클라우드 수신 포트 |
| `lidar[0].driver` | `difop_port` | DIFOP 패킷 수신 포트 |
| `lidar[0].ros` | `ros_frame_id` | 포인트 클라우드 frame ID |
| `lidar[0].ros` | `ros_imu_frame_id` | IMU frame ID (`/tf_static` child frame) |

### 5.2 launch 파라미터

launch 시 인수를 넘겨 config.yaml의 값을 덮어쓸 수 있습니다.

| 인수 | 기본값 | 설명 |
|------|--------|------|
| `lidar_frame_id` | `rslidar` | 포인트 클라우드·TF parent frame |
| `imu_frame_id` | `rslidar_imu` | TF child frame |
| `device_address` | `192.168.1.200` | LiDAR IP 주소 |
| `device_port` | `6699` | LiDAR 제어 포트 |

---

## 6 실행

```sh
ros2 launch rslidar_sdk humble_start.launch.py
```

파라미터를 변경하여 실행하는 예시:

```sh
ros2 launch rslidar_sdk humble_start.launch.py \
  device_address:=192.168.1.100 \
  lidar_frame_id:=lidar_link \
  imu_frame_id:=imu_link
```

---

## 7 주요 토픽 / 서비스

### 7.1 토픽

| 토픽 | 타입 | 설명 |
|------|------|------|
| `/rslidar_sdk/rslidar_points` | `sensor_msgs/PointCloud2` | 포인트 클라우드 |
| `/rslidar_sdk/rslidar_imu_data` | IMU 데이터 | IMU 원시 데이터 |
| `/tf_static` | `tf2_msgs/TFMessage` | LiDAR ↔ IMU extrinsic (DIFOP에서 자동 추출) |

### 7.2 서비스 (`rslidar_ctrl_node` 필요)

| 서비스 | 타입 | 설명 |
|--------|------|------|
| `/rslidar_sdk/rslidar/reboot` | `std_srvs/Trigger` | LiDAR 재부팅 |
| `/rslidar_sdk/rslidar/set_power` | `std_srvs/SetBool` | LiDAR 전원 제어 (`true`=ON, `false`=OFF) |

서비스 호출 예시:

```sh
# 재부팅
ros2 service call /rslidar_sdk/rslidar/reboot std_srvs/srv/Trigger

# 전원 끄기
ros2 service call /rslidar_sdk/rslidar/set_power std_srvs/srv/SetBool "{data: false}"

# 전원 켜기
ros2 service call /rslidar_sdk/rslidar/set_power std_srvs/srv/SetBool "{data: true}"
```

---

## 8 상위 문서 (rslidar_sdk)

- [파라미터 소개](doc/intro/02_parameter_intro.md)
- [온라인 LiDAR 연결 및 포인트 클라우드 수신](doc/howto/06_how_to_decode_online_lidar.md)
- [PCAP 파일 디코딩](doc/howto/08_how_to_decode_pcap_file.md)
- [ROS2 Humble 프레임 드롭 해결](doc/howto/13_how_to_solve_ROS2_humble_frame_rate_drop.md)
