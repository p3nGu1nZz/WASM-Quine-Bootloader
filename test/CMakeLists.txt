# Test targets for WASM Quine Bootloader

# Use Catch2 for unit testing (setup.sh should download & install it)
find_package(Catch2 3.0.0 REQUIRED)

# Simple tests that link against the 'core' library
add_executable(test_state test_state.cpp)
target_include_directories(test_state PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
)
target_link_libraries(test_state PRIVATE
    core
    Catch2::Catch2WithMain
)
add_test(NAME state_test COMMAND test_state)

# additional tests
add_executable(test_util_dpi test_util_dpi.cpp)
# Include ImGui implementation sources so that Gui methods (which call ImGui)
# have their dependencies satisfied without needing the full bootloader.
target_sources(test_util_dpi PRIVATE
    ${IMGUI_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/gui/window.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/heatmap.cpp
)
target_include_directories(test_util_dpi PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
)
target_link_libraries(test_util_dpi PRIVATE
    core
    Catch2::Catch2WithMain
)
add_test(NAME util_dpi_test COMMAND test_util_dpi)

# CLI parsing tests
add_executable(test_cli test_cli.cpp)
target_include_directories(test_cli PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(test_cli PRIVATE
    core
    Catch2::Catch2WithMain
)
add_test(NAME cli_test COMMAND test_cli)
