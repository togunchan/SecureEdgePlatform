cmake_minimum_required(VERSION 3.10)
project(SecureEdgePlatform)
set(CMAKE_CXX_STANDARD 20)

include(CTest)

add_subdirectory(third_party/Catch2 ${CMAKE_BINARY_DIR}/Catch2-build)

add_subdirectory(CppMiniDB)
add_subdirectory(SensorSimulator)
add_subdirectory(EdgeAgent)
add_subdirectory(SecureBootSimulator)
add_subdirectory(EdgeGateway)
add_subdirectory(SecureEdgePlatformController)
add_subdirectory(SecureEdgePlatformRunner)

if(BUILD_TESTING)
    add_executable(test_full_platform
        tests/test_full_platform.cpp
    )
    target_link_libraries(test_full_platform
        PRIVATE
            Catch2::Catch2WithMain
            edgegateway
    )
    include(Catch)
    catch_discover_tests(test_full_platform TEST_PREFIX FullPlatform::)
endif()