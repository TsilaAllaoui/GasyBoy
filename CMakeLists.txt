cmake_minimum_required(VERSION 3.25)

project(gasyboy VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

set(GASYBOY_SOURCES_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridge.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu.cpp

    # ${CMAKE_CURRENT_SOURCE_DIR}/src/eventListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gbException.cpp

    # ${CMAKE_CURRENT_SOURCE_DIR}/src/gameboy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gamepad.cpp

    # ${CMAKE_CURRENT_SOURCE_DIR}/src/gpu.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/interrupt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mmu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/register.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/registers.cpp

    # ${CMAKE_CURRENT_SOURCE_DIR}/src/timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
)

set(GASYBOY_HEADERS_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(${EXTERNALS_DIR}/SDL)
add_subdirectory(${EXTERNALS_DIR}/argparse)
add_subdirectory(${EXTERNALS_DIR}/googletest)

# Add the library target
add_library(gasyboy_lib ${GASYBOY_SOURCES_FILES})

target_include_directories(gasyboy_lib PUBLIC ${GASYBOY_HEADERS_DIR})
target_include_directories(gasyboy_lib PUBLIC ${EXTERNALS_DIR}/SDL/include)
target_include_directories(gasyboy_lib PUBLIC ${EXTERNALS_DIR}/argparse/include/argparse)

target_link_libraries(gasyboy_lib PUBLIC SDL2main SDL2)

# Add the executable target
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_NAME} gasyboy_lib)

# Enable testing
enable_testing()

# Add the tests subdirectory
add_subdirectory(tests)
