cmake_minimum_required(VERSION 3.20)

# Project info setup
project(gasyboy VERSION 2.0 LANGUAGES CXX)

# Compiler norm to use
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# External libs directory
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# Imgui source files
file(GLOB IMGUI_SRC
    "${EXTERNALS_DIR}/imgui/*.cpp"
    "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdl2.cpp"
    "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp"
)

# Add all source files in src/ excluding main.cpp
file(GLOB_RECURSE GASYBOY_SOURCES_FILES "src/*.cpp")
list(REMOVE_ITEM GASYBOY_SOURCES_FILES src/main.cpp)

# Add include directory
set(GASYBOY_HEADERS_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Adding external libs
add_subdirectory(${EXTERNALS_DIR}/SDL)
add_subdirectory(${EXTERNALS_DIR}/argparse)
add_subdirectory(${EXTERNALS_DIR}/gbit)

# Add the library target
add_library(${PROJECT_NAME}_lib ${GASYBOY_SOURCES_FILES} ${IMGUI_SRC})

# Creating executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Adding project include to project lib and expose it
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${GASYBOY_HEADERS_DIR})

# Adding libs includes to project lib
target_include_directories(${PROJECT_NAME}_lib PUBLIC
    ${EXTERNALS_DIR}/SDL/include
    ${EXTERNALS_DIR}/argparse/include/argparse
    ${EXTERNALS_DIR}/imgui
    ${EXTERNALS_DIR}/imgui/backends
)

# Linking other libs to project lib
target_link_libraries(${PROJECT_NAME}_lib PRIVATE SDL2main SDL2 gbit)

# Linking project lib to project executable
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

# Enable testing
enable_testing()

# Add the tests subdirectory
add_subdirectory(tests)
