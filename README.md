# cpp-container-template

## FPS Arena Survival MVP

This repository contains a small C++17 first-person arena-survival game built with raylib.

### Prerequisites

The headless rules tests require `g++`. Building or running the game additionally requires raylib and CMake.

On Ubuntu systems where raylib is not supplied by the configured package repositories, install it from source:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libasound2-dev libgl1-mesa-dev libx11-dev libxcursor-dev libxi-dev libxinerama-dev libxrandr-dev
git clone --depth 1 https://github.com/raysan5/raylib.git /tmp/raylib
cmake -S /tmp/raylib -B /tmp/raylib/build -DBUILD_EXAMPLES=OFF
cmake --build /tmp/raylib/build
sudo cmake --install /tmp/raylib/build
cmake --find-package -DNAME=raylib -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST
```

### Build and run

```bash
./test_runner.sh --unit   # headless game-rules tests
./test_runner.sh --build  # builds build/fps_arena
./test_runner.sh --run    # builds and launches the game
```

Controls: `W/A/S/D` move, arrow keys aim, Space shoots, Enter restarts after death, and Escape exits.

## Getting Started

This repository is compatible with [cpp-container](https://github.com/ChicoState/cpp-container). If not already built on your machine, clone and build it.

Run the container:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container
```

Run the application interactively in a shell:

```bash
docker run -v "$(pwd)":/usr/src -it cpp-container sh
```

## Structure

* `.agents` - AI agent configurations and skills (in `/skills` subdirectory) for this project
* `.` - The root directory contains the C++ code for the application as well as necessary scripts
* `specs` - Specification documentation
* `tests` - Test code
