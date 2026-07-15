add_executable(five-beam-retrieval-test
    FiveBeamWindRetrievalTest.cpp
)

target_link_libraries(five-beam-retrieval-test PRIVATE radar-core Qt6::Core)

add_test(NAME five-beam-retrieval-test COMMAND five-beam-retrieval-test)
set_tests_properties(five-beam-retrieval-test PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:E:/Qt/6.11.1/mingw_64/bin"
)

add_executable(device-service-integration-test
    DeviceServiceIntegrationTest.cpp
)
target_link_libraries(device-service-integration-test PRIVATE radar-core Qt6::Core)

add_executable(wind-field-render-test
    PpiRenderTest.cpp
    ${CMAKE_SOURCE_DIR}/src/client/ui/widgets/WindVectorFieldWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/client/ui/widgets/WindVectorFieldWidget.h
)
target_include_directories(wind-field-render-test PRIVATE ${CMAKE_SOURCE_DIR}/src/client)
target_link_libraries(wind-field-render-test PRIVATE Qt6::Widgets)
add_test(NAME wind-field-render-test COMMAND wind-field-render-test)
set_tests_properties(wind-field-render-test PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:E:/Qt/6.11.1/mingw_64/bin"
)

add_executable(data-service-history-test
    DataServiceHistoryTest.cpp
)
target_link_libraries(data-service-history-test PRIVATE radar-core Qt6::Core Qt6::Sql)
add_test(NAME data-service-history-test COMMAND data-service-history-test)
set_tests_properties(data-service-history-test PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:E:/Qt/6.11.1/mingw_64/bin"
)
