cmake_minimum_required(VERSION 3.20)

set(_radar_temp_dir "${CMAKE_SOURCE_DIR}/.codex_tmp")
file(MAKE_DIRECTORY "${_radar_temp_dir}")
set(ENV{TMP} "${_radar_temp_dir}")
set(ENV{TEMP} "${_radar_temp_dir}")
set(ENV{TMPDIR} "${_radar_temp_dir}")
set(ENV{PATH} "E:/Qt/Tools/mingw1310_64/bin;E:/Qt/Tools/CMake_64/bin;C:/Windows/system32;C:/Windows;C:/Windows/System32/Wbem")

project(RadarUpperComputer VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)
find_package(Qt6 QUIET COMPONENTS WebSockets)
find_package(Qt6 QUIET COMPONENTS Charts)
find_package(Qt6 QUIET COMPONENTS SerialPort)

message(STATUS "=====================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "=====================================")

add_subdirectory(src/core)

option(BUILD_CLIENT "Build desktop client" ON)
if(BUILD_CLIENT)
    message(STATUS "Building: Desktop Client")
    add_subdirectory(src/client)
endif()

option(BUILD_RADAR_WEB "Build radar web server" OFF)
if(BUILD_RADAR_WEB)
    message(STATUS "Building: Radar Web Server")
    add_subdirectory(src/radar-web/server)
endif()

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    message(STATUS "Building: Tests")
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "=====================================")
