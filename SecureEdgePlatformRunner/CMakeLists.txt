add_executable(SecureEdgePlatformRunner
    src/main.cpp
)

target_include_directories(SecureEdgePlatformRunner PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/SecureEdgePlatformController/include>
)

target_link_libraries(SecureEdgePlatformRunner
    PRIVATE platform_controller
)