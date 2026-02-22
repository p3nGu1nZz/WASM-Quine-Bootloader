cmake_minimum_required(VERSION 3.20)
project(WasmQuineBootloader LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── SDL3 ───────────────────────────────────────────────────────────────────────
# Try to find a system-installed SDL3 first; if not found, use the one in external/
find_package(SDL3 QUIET CONFIG)
if(NOT SDL3_FOUND)
    set(SDL3_DIR "${CMAKE_SOURCE_DIR}/external/SDL3/build/lib/cmake/SDL3"
        CACHE PATH "SDL3 CMake config directory")
    find_package(SDL3 REQUIRED CONFIG)
endif()

# ── wasm3 ─────────────────────────────────────────────────────────────────────
set(WASM3_DIR "${CMAKE_SOURCE_DIR}/external/wasm3")
file(GLOB WASM3_SOURCES "${WASM3_DIR}/source/*.c")

# ── ImGui ─────────────────────────────────────────────────────────────────────
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
)

# ── Application sources ───────────────────────────────────────────────────────
set(APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/wasm_kernel.cpp
    src/wasm_parser.cpp
    src/wasm_evolution.cpp
)

add_executable(wasm_quine_bootloader
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
    ${WASM3_SOURCES}
)

target_include_directories(wasm_quine_bootloader PRIVATE
    src/
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${WASM3_DIR}/source"
)

target_link_libraries(wasm_quine_bootloader PRIVATE
    SDL3::SDL3
)

# Compiler warnings (optional but helpful for development)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(wasm_quine_bootloader PRIVATE
        -Wall -Wextra -Wno-unused-parameter
    )
endif()

# Install target
install(TARGETS wasm_quine_bootloader RUNTIME DESTINATION bin)
