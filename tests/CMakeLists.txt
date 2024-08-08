cmake_minimum_required(VERSION 3.25)

# Project info setup
project(gasyboy_tests VERSION 1.0 LANGUAGES CXX)

# Add tests source files
file(GLOB GASYBOY_TEST_SOURCES "*.cpp")

# Add an executable for the tests
add_executable(${PROJECT_NAME} ${GASYBOY_TEST_SOURCES})

# Link GoogleTest libraries and main project lib
target_link_libraries(${PROJECT_NAME} gtest gtest_main gasyboy_lib)

# Add the tests to CTest
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
