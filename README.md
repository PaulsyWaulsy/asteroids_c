![Language](https://img.shields.io/badge/language-C-blue)
![Framework](https://img.shields.io/badge/framework-SDL2-orange)
![Build](https://img.shields.io/badge/build-CMake-informational)
![Status](https://img.shields.io/badge/status-active-success)
![License](https://img.shields.io/badge/license-MIT-green)

## Asteroids — Classic Arcade Game in C

A faithful recreation of the **Asteroids** arcade game, written in **C** using **SDL2** and **CMake**.
All graphics are rendered manually with `SDL_RenderDrawLine`, replicating the original vector-style visuals.
Audio feedback is included for shooting, collisions, and explosions.

## Overview

Asteroids is a minimal and self-contained project that demonstrates core concepts in 2D rendering, vector mathematics, and real-time input handling using SDL2.
The implementation avoids any pre-made assets or engines — everything is drawn and updated procedurally.

## Features

- Multiple asteroid sizes with fragmentation behavior
- Player ship with rotation, thrust, and shooting
- Line-based rendering with no textures or sprites
- Event-based audio for shooting and destruction
- Lightweight CMake build configuration

## Dependencies

- **SDL2** — Rendering and input
- **SDL2_mixer** — Audio playback
- **CMake** — Build configuration
- **gcc** or **clang** — C compiler

## Install on Ubuntu/Debian

```bash
sudo apt update
sudo apt install libsdl2-dev libsdl2-mixer-dev cmake build-essential
```

---

## Build and Run

Clone the repository:

```bash
git clone https://github.com/PaulsyWausly/asteroids_c.git
cd asteroids_c
```

Create a build directory and compile:

```bash
mkdir build
cd build
cmake ..
make
```

Run the program:

```bash
./asteroids
```

## Controls

| Action       | Key      |
| ------------ | -------- |
| Rotate Left  | ← or A   |
| Rotate Right | → or D   |
| Thrust       | ↑ or W   |
| Shoot        | Spacebar |
| Quit         | Esc      |

## Project Structure

```
.
├── build/                # Build output (created by CMake)
│   └── asteroids         # Compiled binary
├── src/
│   ├── main.c            # Game loop and rendering logic
│   ├── vec.c             # Vector math utilities
│   └── vec.h             # Vector structure and helper functions
├── sounds/
│   ├── alien.wav
│   ├── explosion.wav
│   ├── hit.wav
│   ├── random.wav
│   └── shoot.wav
└── CMakeLists.txt        # Build configuration
```

## Technical Notes

- **Rendering**: All visuals use `SDL_RenderDrawLine` for vector-style output.
- **Physics**: Object movement and rotation are handled with simple vector operations.

## Future Improvements

- [ ] Score tracking and UI overlay
- [ ] Screen wrapping indicators
- [ ] Additional sound design and balancing
- [ ] Difficulty scaling and levels
- [ ] Persistent high-score file
