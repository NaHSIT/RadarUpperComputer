cmake_minimum_required(VERSION 3.20)
project(RadarUpperComputer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Charts
    SerialPort
    Network
)

# 源文件
set(SOURCES
    src/main.cpp
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
    Qt6::SerialPort
    Qt6::Network
)
