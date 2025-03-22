cmake_minimum_required(VERSION 3.20)

# Project info setup
project(gasyboy VERSION 2.0 LANGUAGES CXX)

# Default to Release if no build type is specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# Print build type for confirmation
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set compiler flags for different build types
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")       # Debug: No optimizations, full debugging info
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")  # Release: Optimized, disable asserts
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g") # Release with debug info
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG") # Minimized binary size

# Apply flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


# Compiler norm to use
set(CMAKE_CXX_STANDARD 17)

# External libs directory
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# Add all source files in src/ excluding main.cpp
file(GLOB GASYBOY_SOURCES_FILES src/*.cpp src/providers/*.cpp)
list(REMOVE_ITEM GASYBOY_SOURCES_FILES src/main.cpp)

if(USE_EMSCRIPTEN_SDL2 OR(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
    # Add include directory
    set(GASYBOY_HEADERS_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/events
        ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
    )

    # Creating executable
    add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES} src/main.cpp)

    # Add headers to executable
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GASYBOY_HEADERS_DIR}
        ${EXTERNALS_DIR}/argparse/include/argparse)

    # Emscripten SDL setup
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set(SDL2_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -sEXPORTED_FUNCTIONS=_main,_load_file,_toggle_bios,_malloc,_free -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sNO_DISABLE_EXCEPTION_CATCHING")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL2_FLAGS}")

    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)

    # Add the Emscripten specific options
    set_target_properties(${PROJECT_NAME} PROPERTIES

        LINK_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1"
    )
else()
    # Adding debugger sources
    file(GLOB GASYBOY_DEBUGGER_SOURCES_FILES src/debugger/*.cpp)

    # Imgui source files
    file(GLOB IMGUI_SRC
        "${EXTERNALS_DIR}/imgui/*.cpp"
        "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdl2.cpp"
        "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp"
        "${EXTERNALS_DIR}/ImGuiFileDialog/ImGuiFileDialog.cpp"
    )

    # Creating executable
    add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES} ${GASYBOY_DEBUGGER_SOURCES_FILES} ${IMGUI_SRC} src/main.cpp)

    set(GASYBOY_HEADERS_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/debugger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/events
        ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
        ${EXTERNALS_DIR}/ImGuiFileDialog)

    # Add headers to executable
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GASYBOY_HEADERS_DIR}
        ${EXTERNALS_DIR}/argparse/include/argparse
        ${EXTERNALS_DIR}/imgui
        ${EXTERNALS_DIR}/imgui/backends)

    # Non-Emscripten (native) build
    add_subdirectory(externals/SDL)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNALS_DIR}/SDL/include
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main SDL2)
endif()

if(WIN32)
    # If on win32 env, Copy SDL2.dll to the build folder after the executable is built
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/externals/SDL/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
        "${CMAKE_BINARY_DIR}/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
        COMMENT "Copying SDL2 DLL to the build folder"
    )
elseif(USE_EMSCRIPTEN_SDL2 OR(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
    # If on wasm env, Copy generated wasm and js to wasm folder
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/${PROJECT_NAME}.js"

        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/${PROJECT_NAME}.wasm"

        COMMENT "Copying wasm generated files to wasm folder"
    )
endif()
