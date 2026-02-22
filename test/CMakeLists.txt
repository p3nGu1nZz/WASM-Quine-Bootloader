# Test targets for WASM Quine Bootloader

# Simple smoke tests that link against the 'core' library.
add_executable(test_state test_state.cpp)
target_include_directories(test_state PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_state PRIVATE core)
add_test(NAME state_test COMMAND test_state)
