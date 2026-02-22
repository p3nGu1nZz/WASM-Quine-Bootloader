# Run the Application

Run the WASM Quine Bootloader after building it.

## Quick Commands

```bash
# Build (if not already built) and run in terminal/headless mode (default)
bash scripts/run.sh

# Run with SDL3 GUI window
bash scripts/run.sh --gui

# Pass additional flags directly to the bootloader
bash scripts/run.sh --gui --verbose
```

All arguments after `run.sh` are forwarded verbatim to the `bootloader` executable.

## Running Modes

### Terminal Mode (default — no `--gui`)
- The bootloader renders all output to the current terminal using ANSI escape sequences.
- The display refreshes in-place (like `top`) — do **not** use `printf` or `std::cout` for panel output; use the terminal rendering API in `src/gui.cpp`.
- Panels shown: system state, boot log, memory view, instruction stream, kernel metrics.
- Press **Ctrl-C** to exit.

### GUI Mode (`--gui`)
- Opens an SDL3 window, black background, windowed by default.
- **F11** toggles between windowed and fullscreen mode (another press restores windowed).
- All panels mirror the terminal mode layout, rendered to the SDL3 surface.
- Close the window or press **Escape** to exit.

## How `scripts/run.sh` Works

1. Checks whether `build/linux-debug/bootloader` exists.
2. If not, calls `bash scripts/build.sh` automatically.
3. Executes `./build/linux-debug/bootloader "$@"` forwarding all provided arguments.

## Implementing Terminal Mode in `src/main.cpp`

Terminal rendering must clear the screen and redraw on each tick:

```cpp
// Clear screen and move cursor to top-left
std::fputs("\033[2J\033[H", stdout);

// Draw each panel using the Gui class helpers
gui.drawSystemStatus(state);
gui.drawBootLog(log);
gui.drawMemoryView(memory);
// … etc.

std::fflush(stdout);
```

Use `struct termios` (POSIX) or `_getch` (Windows) to read keyboard input without blocking.

## Implementing GUI Mode in `src/gui.cpp`

```cpp
// Window creation (windowed, resizable)
SDL_Window* window = SDL_CreateWindow("WASM Quine Bootloader",
    1280, 720, SDL_WINDOW_RESIZABLE);

// F11 fullscreen toggle inside the event loop
case SDLK_F11:
    isFullscreen = !isFullscreen;
    SDL_SetWindowFullscreen(window, isFullscreen);
    break;
```
