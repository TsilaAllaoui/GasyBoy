cmake_minimum_required(VERSION 3.25)

project(GasyBoy VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

set(GASYBOY_SOURCES_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cartridge.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/cartridge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/cpu.h
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/defs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/eventListener.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/eventListener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gameboy.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/gameboy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gamepad.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/gamepad.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gpu.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/gpu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interrupt.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/interrupt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mmu.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/mmu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/register.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/register.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/timer.h
)

set(GASYBOY_HEADERS_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

message(STATUS "Header directory: ${GASYBOY_HEADERS_DIR}")


add_subdirectory(${EXTERNALS_DIR}/SDL)

add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${GASYBOY_HEADERS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNALS_DIR}/SDL/include)

target_link_libraries(GasyBoy PUBLIC SDL2main SDL2)