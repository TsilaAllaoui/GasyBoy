cmake_minimum_required(VERSION 3.20)

# Project info setup
project(gasyboy VERSION 2.0 LANGUAGES CXX)

# Set compiler standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)  # or 99, depending on what SDL requires


# Default to Release if no build type is specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# Print build type for confirmation
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Ensure correct build directories based on type
set(BUILD_DIR "${CMAKE_BINARY_DIR}/build_${CMAKE_BUILD_TYPE}")
file(MAKE_DIRECTORY ${BUILD_DIR})
message(STATUS "Build directory: ${BUILD_DIR}")

# Set compiler flags based on compiler
if(MSVC)
    # MSVC flags
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od") # Debug: Generate debug symbols, disable optimization
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG") # Release: Optimize, disable asserts
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi") # Release with debug info
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/O1 /DNDEBUG") # Minimized binary size
else()
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0") # Debug: No optimizations, full debugging info
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG") # Release: Optimized, disable asserts
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g") # Release with debug info
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG") # Minimized binary size
endif()

# Apply flags only if using a single-config generator (GCC, Clang)
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|MSVC")
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++20>)
endif()


# External libs directory
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# Add all source files in src/ excluding main.cpp
file(GLOB_RECURSE GASYBOY_SOURCES_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/providers/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/instructions/*.cpp"
)
list(REMOVE_ITEM GASYBOY_SOURCES_FILES src/main.cpp)

# Check if using Emscripten
if(USE_EMSCRIPTEN_SDL2 OR(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
    # Add include directories
    set(GASYBOY_HEADERS_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/events
        ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
    )

    # Creating executable
    add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES} src/main.cpp)

    # Add include directories to executable
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GASYBOY_HEADERS_DIR}
        ${EXTERNALS_DIR}/argparse/include/argparse
    )

    # Emscripten SDL setup
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set(SDL2_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -sEXPORTED_FUNCTIONS=_main,_load_file,_toggle_bios,_malloc,_free -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sNO_DISABLE_EXCEPTION_CATCHING")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL2_FLAGS}")

    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)

    # Emscripten-specific options
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1")

else()
    # Adding debugger sources
    file(GLOB GASYBOY_DEBUGGER_SOURCES_FILES src/debugger/*.cpp)

    # ImGui source files
    file(GLOB IMGUI_SRC
        "${EXTERNALS_DIR}/imgui/*.cpp"
        "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdl2.cpp"
        "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp"
        "${EXTERNALS_DIR}/ImGuiFileDialog/ImGuiFileDialog.cpp"
    )

    # Creating executable
    add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES} ${GASYBOY_DEBUGGER_SOURCES_FILES} ${IMGUI_SRC} src/main.cpp)

    # Set include directories
    set(GASYBOY_HEADERS_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/debugger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/events
        ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
        ${EXTERNALS_DIR}/ImGuiFileDialog
    )

    # Add headers to executable
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GASYBOY_HEADERS_DIR}
        ${EXTERNALS_DIR}/argparse/include/argparse
        ${EXTERNALS_DIR}/imgui
        ${EXTERNALS_DIR}/imgui/backends
    )

    # Non-Emscripten (native) build
    add_subdirectory(externals/SDL)
    target_include_directories(${PROJECT_NAME} PRIVATE ${EXTERNALS_DIR}/SDL/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main SDL2)
endif()

# Handle platform-specific operations
if(WIN32)
    # Windows: Copy SDL2.dll after build
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/externals/SDL/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
        "${CMAKE_BINARY_DIR}/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
        || exit 0
        COMMENT "Copying SDL2 DLL to the build folder (non-fatal)"
    )
elseif(USE_EMSCRIPTEN_SDL2 OR(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
    # WebAssembly: Copy generated wasm/js to docs folder
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/${PROJECT_NAME}.js"
    )

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/${PROJECT_NAME}.wasm"
        COMMENT "Copying WebAssembly-generated files to docs folder"
    )
endif()
