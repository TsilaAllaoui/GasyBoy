cmake_minimum_required(VERSION 3.10)
project(gasyboy_cpu_tester LANGUAGES CXX)

# Compiler norm to use
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source files for the binary
set(GASYBOY_CPU_TESTS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_cpu.cpp
)

# Add sources to executable
add_executable(${PROJECT_NAME} ${GASYBOY_CPU_TESTS_SOURCES})

# Link the shared library to the executable
target_link_libraries(${PROJECT_NAME} PUBLIC
    gbit
    gasyboy_lib
)
