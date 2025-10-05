cmake_minimum_required(VERSION 3.10)
project(SecureBootSimulator)

set(CMAKE_CXX_STANDARD 20)

# === Include paths ===
include_directories(include)

# === Library (main module) ===
add_library(secureboot
    src/BootSimulator.cpp
    src/BootStage.cpp
    src/BootConfig.cpp
    src/SignatureVerifier.cpp
)

target_include_directories(secureboot
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../third_party/json/single_include
)


# === Test executable ===
add_executable(test_secureboot tests/test_bootsimulator.cpp)

# === Include test & third-party headers ===
target_include_directories(test_secureboot 
    PRIVATE 
    tests
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/../third_party/Catch2/src
    ${PROJECT_SOURCE_DIR}/../third_party/json/single_include
)

# === Link the main module and Catch2 ===
target_link_libraries(test_secureboot 
    PRIVATE 
    secureboot 
    Catch2::Catch2WithMain
)

# === Provide absolute path to test data files ===
target_compile_definitions(test_secureboot
    PRIVATE
    DATA_DIR_PATH="${PROJECT_SOURCE_DIR}/tests/data"
)

# === Enable Catch2 test discovery ===
include(CTest)
include(Catch)
catch_discover_tests(test_secureboot)
