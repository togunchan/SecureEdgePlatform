cmake_minimum_required(VERSION 3.10)
project(SensorSimulator)

set(CMAKE_CXX_STANDARD 20)

# --------------------------
# Header-only core library
# --------------------------
add_library(sensor_core INTERFACE)

target_include_directories(sensor_core
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

# --------------------------
# Sensor implementation library (EdgeShell + Scheduler)
# --------------------------
add_library(sensor_impl
    src/cli/EdgeShell.cpp
    src/scheduler/SensorScheduler.cpp
)

target_include_directories(sensor_impl
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(sensor_impl
    PUBLIC
        sensor_core
        minidb
)

# --------------------------
# Sensor tests
# --------------------------
add_executable(test_sensors
    tests/test_sensors.cpp
)

target_include_directories(test_sensors
    PRIVATE
        tests
        ${PROJECT_SOURCE_DIR}/../third_party/Catch2/src
        ${PROJECT_SOURCE_DIR}/SensorSimulator/include
)

target_link_libraries(test_sensors
    PRIVATE
        sensor_core
        Catch2::Catch2WithMain
)

# --------------------------
# CLI tests
# --------------------------
add_executable(test_cli
    tests/test_cli.cpp
)

target_include_directories(test_cli
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/../third_party/Catch2/src
)

target_link_libraries(test_cli
    PRIVATE
        sensor_impl
        Catch2::Catch2WithMain
)

# --------------------------
# Catch2 discovery
# --------------------------
include(CTest)
include(Catch)
catch_discover_tests(test_sensors)
catch_discover_tests(test_cli)

# --------------------------
# Shell executable
# --------------------------
add_executable(EdgeShell
    src/main_shell.cpp
)

target_include_directories(EdgeShell
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(EdgeShell
    PRIVATE
        sensor_impl
)