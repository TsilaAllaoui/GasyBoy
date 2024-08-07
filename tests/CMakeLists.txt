# tests/CMakeLists.txt
# Specify the source files for the tests
file(GLOB TEST_SOURCES "*.cpp")

# Add an executable for the tests
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

# Link GoogleTest libraries
target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main)

# Link the main project library
target_link_libraries(${PROJECT_NAME}_tests gasyboy_lib)

# Add the tests to CTest
add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
