cmake_minimum_required(VERSION 3.20)
project(RadarUpperComputer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt包 (基础组件)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

# 可选组件 (稍后安装)
find_package(Qt6 QUIET COMPONENTS SerialPort)
find_package(Qt6 QUIET COMPONENTS Charts)

# 头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/communication
)

# 源文件
set(SOURCES
    src/main.cpp
    src/communication/protocolparser.cpp
    src/communication/networkmanager.cpp
)

# 头文件
set(HEADERS
    src/communication/frame.h
    src/communication/protocolparser.h
    src/communication/networkmanager.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接Qt库 (基础)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# 链接可选库
if(Qt6SerialPort_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::SerialPort)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_SERIAL_PORT)
endif()

if(Qt6Charts_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Charts)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_CHARTS)
endif()
