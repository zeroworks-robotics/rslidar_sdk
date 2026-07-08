cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)
project(rslidar_sdk)

#=======================================
# Custom Point Type (XYZI,XYZIRT, XYZIF, XYZIRTF)
#=======================================
set(POINT_TYPE XYZI)

option(ENABLE_TRANSFORM "Enable transform functions" OFF)
if(${ENABLE_TRANSFORM})
  add_definitions("-DENABLE_TRANSFORM")
  find_package(Eigen3 REQUIRED)
  include_directories(${EIGEN3_INCLUDE_DIR})
endif(${ENABLE_TRANSFORM})

option(ENABLE_EPOLL_RECEIVE "Receive packets with epoll() instead of select()" OFF)
if(${ENABLE_EPOLL_RECEIVE})
  add_definitions("-DENABLE_EPOLL_RECEIVE")
endif(${ENABLE_EPOLL_RECEIVE})

option(ENABLE_MODIFY_RECVBUF "Enable modify size of RECVBUF" ON)
if(${ENABLE_MODIFY_RECVBUF})
  add_definitions("-DENABLE_MODIFY_RECVBUF")
endif(${ENABLE_MODIFY_RECVBUF})

option(ENABLE_WAIT_IF_QUEUE_EMPTY "Enable waiting for a while in handle thread if the queue is empty" OFF)
if(${ENABLE_WAIT_IF_QUEUE_EMPTY})
  add_definitions("-DENABLE_WAIT_IF_QUEUE_EMPTY")
endif(${ENABLE_WAIT_IF_QUEUE_EMPTY})

option(ENABLE_DIFOP_PARSE "Enable parsing DIFOP Packet" ON)
if(${ENABLE_DIFOP_PARSE})
  add_definitions("-DENABLE_DIFOP_PARSE")
endif(${ENABLE_DIFOP_PARSE})

option(ENABLE_STAMP_WITH_LOCAL "Enable stamp point cloud with local time" OFF)
if(${ENABLE_STAMP_WITH_LOCAL})
  add_definitions("-DENABLE_STAMP_WITH_LOCAL")
endif(${ENABLE_STAMP_WITH_LOCAL})


option(ENABLE_SOURCE_PACKET_LEGACY "Enable ROS Source of MSOP/DIFOP Packet v1.3.x" OFF)
if(${ENABLE_SOURCE_PACKET_LEGACY})
  add_definitions("-DENABLE_SOURCE_PACKET_LEGACY")
endif(${ENABLE_SOURCE_PACKET_LEGACY})

option(ENABLE_IMU_DATA_PARSE           "Enable imu data parse" ON)
if(${ENABLE_IMU_DATA_PARSE})

  message(=============================================================)
  message("-- Enable imu data parse")
  message(=============================================================)

  add_definitions("-DENABLE_IMU_DATA_PARSE")

endif(${ENABLE_IMU_DATA_PARSE})

#========================
# Project details / setup
#========================
set(PROJECT_NAME rslidar_sdk)

add_definitions(-DPROJECT_PATH="${PROJECT_SOURCE_DIR}")

if (CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Release)
  add_definitions(-O3)
endif()

add_definitions(-std=c++17)
add_compile_options(-Wall)

#========================
# Point Type Definition
#========================
if(${POINT_TYPE} STREQUAL "XYZI")
  add_definitions(-DPOINT_TYPE_XYZI)
elseif(${POINT_TYPE} STREQUAL "XYZIRT")
  add_definitions(-DPOINT_TYPE_XYZIRT)
elseif(${POINT_TYPE} STREQUAL "XYZIF")
  add_definitions(-DPOINT_TYPE_XYZIF)
elseif(${POINT_TYPE} STREQUAL "XYZIRTF")
  add_definitions(-DPOINT_TYPE_XYZIRTF)
endif()

message(=============================================================)
message("-- POINT_TYPE is ${POINT_TYPE}")
message(=============================================================)

#========================
# Dependencies Setup
#========================

#ROS#
find_package(roscpp 1.12 QUIET)

if(roscpp_FOUND)

  message(=============================================================)
  message("-- ROS Found. ROS Support is turned On.")
  message(=============================================================)

  add_definitions(-DROS_FOUND)

  find_package(roslib QUIET)
  include_directories(${roscpp_INCLUDE_DIRS} ${roslib_INCLUDE_DIRS})
  set(ROS_LIBS ${roscpp_LIBRARIES} ${roslib_LIBRARIES})

  add_definitions(-DRUN_IN_ROS_WORKSPACE)

  find_package(catkin REQUIRED COMPONENTS
    roscpp
    sensor_msgs
    roslib)

  catkin_package(CATKIN_DEPENDS 
    sensor_msgs 
    roslib)

else(roscpp_FOUND)

  message(=============================================================)
  message("-- ROS Not Found. ROS Support is turned Off.")
  message(=============================================================)

endif(roscpp_FOUND)

#ROS2#
find_package(rclcpp QUIET)

if(rclcpp_FOUND)
  message(=============================================================)
  message("-- ROS2 Found. ROS2 Support is turned On.")
  message(=============================================================)

  add_definitions(-DROS2_FOUND)
  include_directories(${rclcpp_INCLUDE_DIRS})
  set(CMAKE_CXX_STANDARD 14)

  find_package(ament_cmake REQUIRED)
  find_package(sensor_msgs REQUIRED)
  find_package(std_msgs REQUIRED)
  find_package(geometry_msgs REQUIRED)
  find_package(tf2_ros REQUIRED)
  find_package(std_srvs REQUIRED)
  find_package(ament_index_cpp REQUIRED)
  find_package(rosidl_default_generators REQUIRED)

  # rslidar_msg 를 별도 colcon 패키지가 아니라 서브모듈로 인라인 빌드한다.
  # (colcon 은 패키지 하위 디렉터리를 스캔하지 않으므로 thirdparty/ 중첩 상태에서
  #  find_package(rslidar_msg) 는 실패한다. add_subdirectory 로 직접 생성한다.)
  add_subdirectory(thirdparty/rslidar_msg)

else(rclcpp_FOUND)
  message(=============================================================)
  message("-- ROS2 Not Found. ROS2 Support is turned Off.")
  message(=============================================================)
endif(rclcpp_FOUND)

#Others#
find_package(yaml-cpp REQUIRED)

#Include directory#
include_directories(${PROJECT_SOURCE_DIR}/src)

#Driver core#
add_subdirectory(thirdparty/rs_driver)
find_package(rs_driver REQUIRED)
include_directories(${rs_driver_INCLUDE_DIRS})

#========================
# Build Setup
#========================

add_executable(rslidar_sdk_node
               node/rslidar_sdk_node.cpp
               src/manager/node_manager.cpp)

target_link_libraries(rslidar_sdk_node                   
                      ${YAML_CPP_LIBRARIES}
                      ${rs_driver_LIBRARIES})

#Ros#
if(roscpp_FOUND)

  target_link_libraries(rslidar_sdk_node 
    ${ROS_LIBS})

  install(TARGETS rslidar_sdk_node
          RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
          LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
          ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})

endif(roscpp_FOUND)

#Ros2#
if(rclcpp_FOUND)

  ament_target_dependencies(rslidar_sdk_node
    rclcpp
    std_msgs
    sensor_msgs
    geometry_msgs
    tf2_ros)

  # 인라인 빌드된 rslidar_msg 의 타입서포트 타깃을 링크 (find_package 대체)
  rosidl_get_typesupport_target(rslidar_msg_typesupport
    rslidar_msg rosidl_typesupport_cpp)
  target_link_libraries(rslidar_sdk_node ${rslidar_msg_typesupport})

  #========================
  # rslidar_ctrl_node (lib/rs_ctrl_driver 소스가 있을 때만 빌드)
  #========================
  set(CTRL_DRIVER_DIR "${PROJECT_SOURCE_DIR}/lib/rs_ctrl_driver")

  if(EXISTS "${CTRL_DRIVER_DIR}/src/lidar_ctrl_driver.cpp")

    message(=============================================================)
    message("-- rs_ctrl_driver sources found. Building rslidar_ctrl_node.")
    message(=============================================================)

    # 아키텍처 종속 정적 라이브러리(x86-64 .a) 대신 소스를 직접 컴파일한다.
    # 이렇게 하면 Jetson(aarch64) 등 어떤 타깃에서도 네이티브로 빌드된다.
    # 의존성은 시스템 Boost 헤더(Asio/System/DateTime, 1.69+ 헤더 온리) + pthread.
    find_package(Boost REQUIRED)

    file(GLOB RS_CTRL_DRIVER_SRCS "${CTRL_DRIVER_DIR}/src/*.cpp")

    add_executable(rslidar_ctrl_node
                   node/rslidar_ctrl_node.cpp
                   ${RS_CTRL_DRIVER_SRCS})

    target_include_directories(rslidar_ctrl_node PRIVATE
      ${CTRL_DRIVER_DIR}
      ${Boost_INCLUDE_DIRS})

    target_link_libraries(rslidar_ctrl_node
      ${YAML_CPP_LIBRARIES}
      pthread)

    ament_target_dependencies(rslidar_ctrl_node
      rclcpp
      std_srvs
      ament_index_cpp)

    install(TARGETS rslidar_ctrl_node
      DESTINATION lib/${PROJECT_NAME})

  else()
    message(WARNING "rs_ctrl_driver sources not found in ${CTRL_DRIVER_DIR}/src. "
                    "rslidar_ctrl_node will NOT be built. "
                    "See lib/rs_ctrl_driver/README.md for setup instructions.")
  endif()

  install(TARGETS
    rslidar_sdk_node
    DESTINATION lib/${PROJECT_NAME})

  install(DIRECTORY
    launch
    rviz
    config
    DESTINATION share/${PROJECT_NAME})

  ament_package()

endif(rclcpp_FOUND)

