# 顶层 CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(RadarUpperComputer VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt 包
find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets WebSockets)

# 可选组件
find_package(Qt6 QUIET COMPONENTS Charts)
find_package(Qt6 QUIET COMPONENTS SerialPort)

# 输出项目信息
message(STATUS "=====================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "=====================================")

# 共享核心库
add_subdirectory(src/core)

# 桌面客户端
option(BUILD_CLIENT "Build desktop client" ON)
if(BUILD_CLIENT)
    message(STATUS "Building: Desktop Client")
    add_subdirectory(src/client)
endif()

# 雷达端 Web 服务
option(BUILD_RADAR_WEB "Build radar web server" OFF)
if(BUILD_RADAR_WEB)
    message(STATUS "Building: Radar Web Server")
    add_subdirectory(src/radar-web/server)
endif()

# 测试
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    message(STATUS "Building: Tests")
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "=====================================")
