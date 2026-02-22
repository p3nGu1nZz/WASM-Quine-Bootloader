cmake_minimum_required(VERSION 3.20)
project(WasmQuineBootloader LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── SDL3 ───────────────────────────────────────────────────────────────────────
# Try to find a system-installed SDL3 first; if not found use external/SDL3
# automatically add our external build path to prefix search paths
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/SDL3/build/lib/cmake/SDL3")
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
    src/gui.cpp
    src/heatmap.cpp
    src/util.cpp
    src/logger.cpp
    src/fsm.cpp
    src/exporter.cpp
    src/app.cpp
    src/wasm/wasm_kernel.cpp
    src/wasm/wasm_parser.cpp
    src/wasm/wasm_evolution.cpp
)

add_executable(bootloader
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
    ${WASM3_SOURCES}
)

target_include_directories(bootloader PRIVATE
    src/
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${WASM3_DIR}/source"
)

target_link_libraries(bootloader PRIVATE
    SDL3::SDL3
)

# ── Windows (MinGW cross-compile) – static runtime to ship single .exe ────────
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_options(bootloader PRIVATE
        -static-libgcc
        -static-libstdc++
        -static
    )
    target_link_libraries(bootloader PRIVATE
        mingw32
        winmm
        ole32
        oleaut32
        imm32
        version
        setupapi
    )
endif()

# ── Compiler warnings ──────────────────────────────────────────────────────────
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bootloader PRIVATE
        -Wall -Wextra -Wno-unused-parameter
    )
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS bootloader RUNTIME DESTINATION bin)
