set(CLIENT_SOURCES
    main.cpp
    app/ClientApp.cpp
    ui/MainWindow.cpp
    ui/navigation/NavigationBar.cpp
    ui/pages/DashboardPage.cpp
    ui/pages/WindFieldPage.cpp
    ui/pages/BeamPage.cpp
    ui/pages/SpectrumPage.cpp
    ui/pages/DeviceHealthPage.cpp
    ui/pages/SettingsPage.cpp
    ui/widgets/MetricCard.cpp
    ui/widgets/WindTrendChart.cpp
    ui/widgets/WindRoseWidget.cpp
    ui/widgets/BeamHealthGrid.cpp
    ui/widgets/AlarmList.cpp
    ui/widgets/RangeGateTable.cpp
)

set(CLIENT_HEADERS
    app/ClientApp.h
    ui/MainWindow.h
    ui/navigation/NavigationBar.h
    ui/pages/DashboardPage.h
    ui/pages/WindFieldPage.h
    ui/pages/BeamPage.h
    ui/pages/SpectrumPage.h
    ui/pages/DeviceHealthPage.h
    ui/pages/SettingsPage.h
    ui/widgets/MetricCard.h
    ui/widgets/WindTrendChart.h
    ui/widgets/WindRoseWidget.h
    ui/widgets/BeamHealthGrid.h
    ui/widgets/AlarmList.h
    ui/widgets/RangeGateTable.h
)

add_executable(radar-client
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
)

target_include_directories(radar-client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/core
)

target_link_libraries(radar-client
    PRIVATE
        radar-core
        Qt6::Widgets
)

if(TARGET Qt6::Charts)
    target_link_libraries(radar-client PRIVATE Qt6::Charts)
    target_compile_definitions(radar-client PRIVATE HAS_CHARTS)
endif()

set_target_properties(radar-client PROPERTIES
    AUTOMOC ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
