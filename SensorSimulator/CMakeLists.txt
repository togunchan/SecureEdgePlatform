cmake_minimum_required(VERSION 3.10)
project(SensorSimulator)

set(CMAKE_CXX_STANDARD 17)

# Include headers 
include_directories(include)

# Header-only core library
add_library(sensor_core INTERFACE)

target_include_directories(sensor_core
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(test_sensors
    tests/test_sensors.cpp
)

target_include_directories(test_sensors
    PRIVATE
        tests
        ${PROJECT_SOURCE_DIR}/../third_party/Catch2/src
        ${PROJECT_SOURCE_DIR}/SensorSimulator/include
)

# Link dependencies
target_link_libraries(test_sensors
    PRIVATE
        sensor_core
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(test_sensors)

# CLI executable
add_executable(EdgeCLI
    src/cli/EdgeCLI.cpp
)

target_include_directories(EdgeCLI
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(EdgeCLI
    PRIVATE
        sensor_core
)