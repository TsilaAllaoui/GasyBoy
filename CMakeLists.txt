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
add_executable(${PROJECT_NAME} ${GASYBOY_SOURCES_FILES} src/main.cpp)

# Add headers to executable
target_include_directories(${PROJECT_NAME} PRIVATE ${GASYBOY_HEADERS_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EXTERNALS_DIR}/argparse/include/argparse)

# Adding libs includes to project lib
if (USE_EMSCRIPTEN_SDL2 OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
    # Emscripten SDL setup
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(SDL2_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -sEXPORTED_FUNCTIONS=_main,_load_file,_malloc,_free -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sNO_DISABLE_EXCEPTION_CATCHING")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL2_FLAGS}")

    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)

    # Ensure the preload files are included
    set(PRELOAD_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/TETRIS.gb
    )

    # Copy preload files to the build directory
    foreach(FILE ${PRELOAD_FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${FILE}
                ${CMAKE_BINARY_DIR}/$<CONFIG>/${FILENAME}
            COMMENT "Copying ${FILE} to build directory"
        )
    endforeach()

    # Add the Emscripten specific options
    set(PRELOAD_FLAGS "")
    foreach(FILE ${PRELOAD_FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        list(APPEND PRELOAD_FLAGS "--preload-file ${FILENAME}")
    endforeach()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1"
        LINK_FLAGS "-s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 ${PRELOAD_FLAGS}"
    )
else()
    # Non-Emscripten (native) build
    add_subdirectory(externals/SDL)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNALS_DIR}/SDL/include
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
elseif (USE_EMSCRIPTEN_SDL2 OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten"))
    # If on wasm env, Copy generated wasm and js to wasm folder
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js" 
        "${CMAKE_CURRENT_SOURCE_DIR}/wasm/${PROJECT_NAME}.js" 

        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm"
        "${CMAKE_CURRENT_SOURCE_DIR}/wasm/${PROJECT_NAME}.wasm" 
        
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.data"
        "${CMAKE_CURRENT_SOURCE_DIR}/wasm/${PROJECT_NAME}.data"

        COMMENT "Copying wasm generated files to wasm folder"
    )
endif()
