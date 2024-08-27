cmake_minimum_required(VERSION 3.20)

# Project info setup
project(gasyboy VERSION 2.0 LANGUAGES CXX)

# Compiler norm to use
set(CMAKE_CXX_STANDARD 17)

# External libs directory
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# Add all source files in src/ excluding main.cpp
file(GLOB_RECURSE GASYBOY_SOURCES_FILES src/*.cpp)
list(REMOVE_ITEM GASYBOY_SOURCES_FILES src/main.cpp)

# Add include directory
set(GASYBOY_HEADERS_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Creating executable
add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES})

# Add headers to executable
target_include_directories(${PROJECT_NAME} PRIVATE ${GASYBOY_HEADERS_DIR})

# Adding libs includes to project lib
if (USE_EMSCRIPTEN_SDL2)
    # Emscripten SDL setup
    set(SDL2_FLAGS "-sUSE_SDL=2")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL2_FLAGS}")
else()
    # Non-Emscripten (native) build
    add_subdirectory(externals/SDL)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNALS_DIR}/SDL/include
        ${EXTERNALS_DIR}/argparse/include/argparse
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main SDL2)
endif()

if (WIN32)
    # If on win32 env, Copy SDL2.dll to the build folder after the executable is built
    add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/externals/SDL/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
        "${CMAKE_BINARY_DIR}/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
    COMMENT "Copying SDL2 DLL to the build folder"
)

endif()


# em++ -o gasyboy.html \
#     src/*.cpp \
#     -Iexternals/argparse/include/argparse \
#     -s USE_SDL=2 \
#     -s WASM=1

#     em++ -o gasyboy.html     src/*.cpp     -Iexternals/argparse/include/argparse     -s USE_SDL=2     -s WASM=1     -s STACK_SIZE=128KB     -s ALLOW_MEMORY_GROWTH=1