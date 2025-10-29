add_library(platform_controller
    src/SecureEdgePlatformController.cpp
    src/shell/PlatformShell.cpp
)

target_include_directories(platform_controller 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/SensorSimulator/include

)

target_link_libraries(platform_controller
    PUBLIC edgegateway edge_agent sensor_impl secureboot minidb
)
