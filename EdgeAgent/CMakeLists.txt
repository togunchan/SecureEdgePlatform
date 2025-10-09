add_library(edge_agent
    src/TelemetryPublisher.cpp
)

target_include_directories(edge_agent PUBLIC
    ${PROJECT_SOURCE_DIR}/EdgeAgent/include
    ${PROJECT_SOURCE_DIR}/CppMiniDB/include
    ${PROJECT_SOURCE_DIR}/third_party/json/single_include
)

target_link_libraries(edge_agent
)

target_compile_features(edge_agent PRIVATE cxx_std_20)

add_executable(test_telemetry
    tests/test_telemetry.cpp
)

target_include_directories(test_telemetry PRIVATE
    ${PROJECT_SOURCE_DIR}/third_party/json/single_include 
)

target_link_libraries(test_telemetry PRIVATE
    edge_agent
)

target_compile_features(test_telemetry PRIVATE cxx_std_20)