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

# Add a custom command to copy the binary after the build
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}/../build/Debug/
    COMMENT "Copying ${PROJECT_NAME} binary to ${CMAKE_CURRENT_SOURCE_DIR}/../build/Debug/"
)