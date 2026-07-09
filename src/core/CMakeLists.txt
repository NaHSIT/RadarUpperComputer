# 核心库 CMakeLists.txt

set(CORE_SOURCES
    # 领域模型
    domain/RadarDevice.cpp
    domain/BeamState.cpp
    domain/RangeGate.cpp
    domain/WindProfile.cpp
    domain/DeviceHealth.cpp
    domain/AlarmEvent.cpp

    # 服务层
    services/DeviceService.cpp
    services/AlarmService.cpp
    services/DataService.cpp

    # 通信层
    communication/FrameParser.cpp
    communication/TcpDataSource.cpp
    communication/WebSocketManager.cpp
)

set(CORE_HEADERS
    # 领域模型
    domain/RadarTypes.h
    domain/RadarDevice.h
    domain/BeamState.h
    domain/RangeGate.h
    domain/WindProfile.h
    domain/DeviceHealth.h
    domain/AlarmEvent.h

    # 服务层
    services/DeviceService.h
    services/AlarmService.h
    services/DataService.h

    # 通信层
    communication/FrameTypes.h
    communication/FrameParser.h
    communication/TcpDataSource.h
    communication/WebSocketManager.h
)

# 创建静态库
add_library(radar-core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# 设置包含目录
target_include_directories(radar-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接 Qt 库
target_link_libraries(radar-core
    PUBLIC
        Qt6::Core
        Qt6::Network
)

# 可选：WebSockets
if(TARGET Qt6::WebSockets)
    target_link_libraries(radar-core PUBLIC Qt6::WebSockets)
    target_compile_definitions(radar-core PUBLIC HAS_WEBSOCKETS)
endif()

# 设置 C++ 标准
set_target_properties(radar-core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 添加 MOC 支持
set_target_properties(radar-core PROPERTIES AUTOMOC ON)
