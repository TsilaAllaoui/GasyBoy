cmake_minimum_required(VERSION 3.20)

# Project info
project(gasyboy VERSION 2.0 LANGUAGES CXX)

# Options
option(GENERATE_WASM_DEBUG_MAP "Generate .wasm debug map using Emscripten (-g -gsource-map)" OFF)

# Compiler standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/O1 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
endif()

# External paths
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# Gather sources
file(GLOB_RECURSE GASYBOY_SOURCES_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/instructions/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/providers/*.cpp"
)
list(REMOVE_ITEM GASYBOY_SOURCES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Emscripten build
if(USE_EMSCRIPTEN_SDL2 OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))

    file(GLOB_RECURSE DEBUGGER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/debugger/*.cpp")
    list(REMOVE_ITEM GASYBOY_SOURCES_FILES ${DEBUGGER_FILES})

    set(GASYBOY_HEADERS_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/events
        ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
        ${CMAKE_CURRENT_SOURCE_DIR}/src/instructions
    )

    add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES} src/main.cpp)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GASYBOY_HEADERS_DIR}
        ${EXTERNALS_DIR}/argparse/include/argparse
    )

    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    set(SDL2_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1")
    set(SDL2_FLAGS "${SDL2_FLAGS} -sEXPORTED_FUNCTIONS=_main,_load_file,_toggle_bios,_malloc,_free")
    set(SDL2_FLAGS "${SDL2_FLAGS} -sEXPORTED_RUNTIME_METHODS=ccall,cwrap")
    set(SDL2_FLAGS "${SDL2_FLAGS} -sNO_DISABLE_EXCEPTION_CATCHING")

    if(GENERATE_WASM_DEBUG_MAP)
        message(STATUS "Emscripten debug map enabled: Adding -g -gsource-map")
        set(SDL2_FLAGS "${SDL2_FLAGS} -g -gsource-map")
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL2_FLAGS}")
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${SDL2_FLAGS}")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/${PROJECT_NAME}.js"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/${PROJECT_NAME}.wasm"
        COMMENT "Copying WebAssembly-generated files to docs folder"
    )

else()
    # Native build with debugger & ImGui
    file(GLOB GASYBOY_DEBUGGER_SOURCES_FILES src/debugger/*.cpp)
    file(GLOB IMGUI_SRC
        "${EXTERNALS_DIR}/imgui/*.cpp"
        "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdl2.cpp"
        "${EXTERNALS_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp"
        "${EXTERNALS_DIR}/ImGuiFileDialog/ImGuiFileDialog.cpp"
    )

    add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES} ${GASYBOY_DEBUGGER_SOURCES_FILES} ${IMGUI_SRC} src/main.cpp)

    set(GASYBOY_HEADERS_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/debugger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/events
        ${CMAKE_CURRENT_SOURCE_DIR}/src/providers
        ${CMAKE_CURRENT_SOURCE_DIR}/src/instructions
        ${EXTERNALS_DIR}/ImGuiFileDialog
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GASYBOY_HEADERS_DIR}
        ${EXTERNALS_DIR}/argparse/include/argparse
        ${EXTERNALS_DIR}/imgui
        ${EXTERNALS_DIR}/imgui/backends
    )

    add_subdirectory(externals/SDL)
    target_include_directories(${PROJECT_NAME} PRIVATE ${EXTERNALS_DIR}/SDL/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main SDL2)

    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/externals/SDL/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
            "${CMAKE_BINARY_DIR}/$<CONFIG>/$<IF:$<CONFIG:Debug>,SDL2d.dll,SDL2.dll>"
            || exit 0
            COMMENT "Copying SDL2 DLL to the build folder (non-fatal)"
        )
    endif()
endif()

### Debug wasm build
# em++ -std=c++20 -O0 -g -gsource-map -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s EXPORTED_FUNCTIONS="['_main','_load_file','_toggle_bios','_malloc','_free']" -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" -s NO_DISABLE_EXCEPTION_CATCHING -I./src -I./externals/imgui -I./externals/argparse/include/argparse -I./externals/ImGuiFileDialog -I./src/providers src/cartridge.cpp src/cpu.cpp src/debugger/debugger.cpp src/debugger/disassembler.cpp src/gameboy.cpp src/gamepad.cpp src/gbException.cpp src/instructions/adc.cpp src/instructions/add.cpp src/instructions/and.cpp src/instructions/bit.cpp src/instructions/call.cpp src/instructions/ccf.cpp src/instructions/cp.cpp src/instructions/cpl.cpp src/instructions/daa.cpp src/instructions/dec.cpp src/instructions/di.cpp src/instructions/ei.cpp src/instructions/halt.cpp src/instructions/inc.cpp src/instructions/jmp.cpp src/instructions/ld.cpp src/instructions/nop.cpp src/instructions/or.cpp src/instructions/ret.cpp src/instructions/rl.cpp src/instructions/rr.cpp src/instructions/rst.cpp src/instructions/sbc.cpp src/instructions/scf.cpp src/instructions/sr.cpp src/instructions/sub.cpp src/instructions/swap.cpp src/instructions/xor.cpp src/interruptManager.cpp src/logger.cpp src/main.cpp src/mbc.cpp src/mmu.cpp src/ppu.cpp src/providers/cpuProvider.cpp src/providers/gameBoyProvider.cpp src/providers/gamepadProvider.cpp src/providers/interruptManagerProvider.cpp src/providers/mmuProvider.cpp src/providers/ppuProvider.cpp src/providers/registersProvider.cpp src/providers/timerProvider.cpp src/providers/utilitiesProvider.cpp src/register.cpp src/registers.cpp src/renderer.cpp src/timer.cpp src/utils.cpp -o project.js