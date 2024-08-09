cmake_minimum_required(VERSION 3.10)
project(gasyboy_cpu_tester LANGUAGES CXX)

# Compiler norm to use
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# External libs directory
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../externals)

# Source files for gbit lib
file(GLOB_RECURSE GBIT_LIB_SOURCES "${EXTERNALS_DIR}/gbit/*.c")

# Create lib from gbit
add_library(gbit_lib ${GBIT_LIB_SOURCES})

# Source files for the binary
file(GLOB_RECURSE GASYBOY_CPU_TESTS_SOURCES "/*.cpp")

# Add sources to executable
add_executable(${PROJECT_NAME} ${GASYBOY_CPU_TESTS_SOURCES})

# Add external lib include
target_include_directories(${PROJECT_NAME} PRIVATE ${EXTERNALS_DIR}/gbit/lib)

# Link the shared library to the executable
target_link_libraries(${PROJECT_NAME} PRIVATE
    gbit_lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build/Debug/gasyboy_lib.lib
)

