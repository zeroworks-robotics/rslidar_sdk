# rs_ctrl_driver

RoboSense LiDAR control driver (proprietary binary library).

Place the following files in this directory to enable `rslidar_ctrl_node` build:

| File | Description |
|------|-------------|
| `lidar_ctrl_driver.hpp` | Main driver header |
| `ctrl_driver_param.hpp` | Parameter struct definitions |
| `types.h` | Type definitions |
| `librs_ctrl_driver.a` | Static library (Linux x86_64) |
| `libboost_system.a` | Bundled Boost.System (non-PIE build) |

Source: RoboSense linux-lib-namespace package.

If the files are absent, `rslidar_ctrl_node` is silently skipped during build.
