cmake_minimum_required(VERSION 3.14)
project(cpp_method_wrapper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(wrapper_demo
    main.cpp
    wrapper.h
)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_wrapper
    test_wrapper_gtest.cpp
    wrapper.h
)

target_link_libraries(test_wrapper
    GTest::gtest
    GTest::gtest_main
)

enable_testing()
add_test(NAME wrapper_tests COMMAND test_wrapper)
