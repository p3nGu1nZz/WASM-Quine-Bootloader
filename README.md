<div align="center">
<img width="1200" height="475" alt="GHBanner" src="https://github.com/user-attachments/assets/0aa67016-6eaf-458a-adb2-6e31a0763ed6" />
</div>

# WASM Quine Bootloader

A self-replicating, self-evolving WebAssembly kernel visualizer — native C++17
desktop application built with **SDL3** and **Dear ImGui**.

For full documentation see **[docs/README.md](docs/README.md)**.

## Quick Start (Windows WSL2 / Ubuntu)

```bash
# Install all dependencies, build, and run:
bash scripts/setup.sh
bash scripts/run.sh
```

## Build Targets

```bash
bash scripts/build.sh                    # linux-debug (default)
bash scripts/build.sh linux-release
bash scripts/build.sh windows-debug     # requires setup.sh windows first
bash scripts/build.sh windows-release   # requires setup.sh windows first
```

## Controls

| Key / Button | Action |
|---|---|
| `Space` | Pause / Resume |
| `Q` / `Esc` | Quit |
| **EXPORT** button | Save telemetry report to disk |
