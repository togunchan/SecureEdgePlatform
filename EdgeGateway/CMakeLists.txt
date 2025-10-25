add_library(edgegateway
    src/ConsoleChannel.cpp
    src/FileChannel.cpp
    src/main_gateway.cpp
    src/SensorLogRow.cpp
    src/GatewayConfig.cpp
    src/EdgeGateway.cpp
    src/AgentChannel.cpp
)

target_include_directories(edgegateway PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/EdgeAgent/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/SensorSimulator/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/CppMiniDB/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party/json/include>  
)

target_link_libraries(edgegateway
    PUBLIC
        minidb
        sensor_impl
        edge_agent
)

add_executable(EdgeGatewayRunner src/main_gateway.cpp)
target_link_libraries(EdgeGatewayRunner PUBLIC edgegateway)

add_executable(test_edgegateway
    tests/test_gateway.cpp
)

target_link_libraries(test_edgegateway
    PRIVATE
        Catch2WithMain
        edgegateway
)

if(BUILD_TESTING)
    include(Catch)
    catch_discover_tests(test_edgegateway TEST_PREFIX EdgeGateway::)
endif()
