# Test targets for WASM Quine Bootloader

# Use Catch2 for unit testing (setup.sh should download & install it)
find_package(Catch2 3.0.0 REQUIRED)

# Simple tests that link against the 'core' library
add_executable(test_state test_state.cpp)
target_include_directories(test_state PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_state PRIVATE
    core
    Catch2::Catch2WithMain
)
add_test(NAME state_test COMMAND test_state)
