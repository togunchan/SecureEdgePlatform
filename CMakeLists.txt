cmake_minimum_required(VERSION 3.10)
project(SecureEdgePlatform)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(third_party/Catch2 ${CMAKE_BINARY_DIR}/Catch2-build)

add_subdirectory(CppMiniDB)
add_subdirectory(SensorSimulator)

include(CTest)