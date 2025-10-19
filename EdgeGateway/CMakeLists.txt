add_library(edgegateway
    src/ConsoleChannel.cpp
    src/FileChannel.cpp
    src/main_gateway.cpp
    src/SensorLogRow.cpp
    src/GatewayConfig.cpp
    src/EdgeGateway.cpp
)

target_include_directories(edgegateway PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/EdgeAgent/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party/json/include>  
)

target_link_libraries(edgegateway
    PUBLIC minidb
)

add_executable(EdgeGatewayRunner src/main_gateway.cpp)
target_link_libraries(EdgeGatewayRunner PUBLIC edgegateway minidb)
