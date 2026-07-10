set(CORE_SOURCES
    domain/RadarDevice.cpp
    domain/BeamState.cpp
    domain/RangeGate.cpp
    domain/WindProfile.cpp
    domain/DeviceHealth.cpp
    domain/AlarmEvent.cpp
    services/DeviceService.cpp
    services/AlarmService.cpp
    services/DataService.cpp
    communication/FrameParser.cpp
    communication/TcpDataSource.cpp
    communication/WebSocketManager.cpp
)

set(CORE_HEADERS
    domain/RadarTypes.h
    domain/RadarDevice.h
    domain/BeamState.h
    domain/RangeGate.h
    domain/WindProfile.h
    domain/DeviceHealth.h
    domain/AlarmEvent.h
    services/DeviceService.h
    services/AlarmService.h
    services/DataService.h
    communication/FrameTypes.h
    communication/FrameParser.h
    communication/TcpDataSource.h
    communication/WebSocketManager.h
)

add_library(radar-core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_include_directories(radar-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(radar-core
    PUBLIC
        Qt6::Core
        Qt6::Network
)

if(TARGET Qt6::WebSockets)
    target_link_libraries(radar-core PUBLIC Qt6::WebSockets)
    target_compile_definitions(radar-core PUBLIC HAS_WEBSOCKETS)
endif()

set_target_properties(radar-core PROPERTIES
    AUTOMOC ON
)
