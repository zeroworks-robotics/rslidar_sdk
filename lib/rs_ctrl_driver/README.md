# rs_ctrl_driver

RoboSense LiDAR 전원/재부팅 제어 드라이버. `rslidar_ctrl_node` 가 사용합니다.

이전에는 사전 컴파일된 정적 라이브러리(`librs_ctrl_driver.a`, `libboost_system.a`)를
링크했으나, 그 `.a` 는 **x86-64 전용**이라 Jetson(aarch64) 등에서 링크가 실패했습니다.
따라서 **소스를 직접 컴파일**하도록 변경했습니다. (아키텍처 자동 대응)

## 구성

```
lib/rs_ctrl_driver/
├── src/                      # 컴파일되는 소스 (7개)
│   ├── lidar_ctrl_driver.cpp
│   ├── lidar_base.cpp
│   ├── tcp_com.cpp
│   ├── airy_com.cpp
│   ├── helios_com.cpp
│   ├── mems_com.cpp
│   └── ruby4_com.cpp
├── lidar_ctrl_driver.hpp     # 공개 API 헤더 (노드가 include)
├── ctrl_driver_param.hpp
├── types.h
└── *.h                       # 내부 헤더 (tcp_com, lidar_base, airy/helios/mems/ruby4 등)
```

원본: RoboSense `linux-lib-namespace` 패키지.

## 의존성

시스템 Boost 헤더가 필요합니다 (Asio / System / DateTime — Boost 1.69+ 에서 헤더 온리).

```sh
sudo apt-get install -y libboost-dev libboost-system-dev libboost-date-time-dev
```

`src/` 소스가 없으면 CMake 경고를 출력하고 `rslidar_ctrl_node` 빌드를 건너뜁니다.

> ⚠️ 아키텍처 종속 정적 라이브러리(`*.a`)는 다시 커밋하지 마세요. `.gitignore` 로 차단합니다.
