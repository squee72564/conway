# Conway (Sparse Game of Life)

A minimal C++/SFML implementation of Conway's Game of Life using a sparse cell set.

## Features
- Sparse representation (stores only live cells)
- Click/drag to paint cells
- Right-drag to pan the view
- Pause/resume simulation

## Controls
- Left mouse: paint cells (drag)
- Right mouse: pan view (drag)
- P: pause/resume
- Esc: quit

## Build Requirements
- CMake
- C++ compiler with C++20 support
- vcpkg (for SFML 3.x)

## Build
```sh
cmake -S . -B build
cmake --build build
./build/conway
```

## Notes on Performance
This uses a sparse hash-based grid, so large empty areas are cheap. Performance scales with the number of live cells and their active neighborhoods.

## Project Structure
- `src/main.cpp`: app + simulation loop
- `CMakeLists.txt`: build config
- `vcpkg.json`: dependencies
