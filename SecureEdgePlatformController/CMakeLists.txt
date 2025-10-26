 add_library(platform_controller
    src/SecureEdgePlatformController.cpp
)

target_include_directories(platform_controller PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(platform_controller
    PUBLIC edgegateway edge_agent sensor_impl secureboot minidb
)