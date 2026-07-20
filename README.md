# Car Simulator 

## Overview
Top-down 2D car simulator: **car body + 4 wheels**, meters-first physics with a **fixed timestep**, and smooth rendering via a single **unit-quad** mesh and a **RectShader** (scale → rotate → translate).

## Features

- Real-time 2D rendering (unit quad mesh + shader: scale → rotate → translate)
- Kinematic bicycle model (meters + radians)
- Discrete action space (combined accelerate + steer)
- Parking environment scaffolding (for future RL)
- CMake build + optional tests
- CI workflow (GitHub Actions)

---

## Simulation Environment
Windows 11

### Library
| Library      | version | link |
|-----------|---------|---------| 
| GLFW    | 3.4 | https://www.glfw.org/download.html, https://github.com/glfw/glfw |
| GLAD | APIs: gl=4.6   | https://glad.dav1d.de/, https://github.com/dav1dde/glad, https://rpxomi.github.io/ |


### Controls
A discrete action space is currently implemented and the car movement is calculated by a kinematic bicycle model with the input controls. 
Combined actions (e.g. accelerate + steer) are possible.
| Controls      | Description |
|-----------|---------| 
| Up | +acceleration |
| Down | -acceleration |
| Left | +steer(CCW) |
| Right | -steer(CW) |
| Escape | Quit |


## Build System

The necessary tools for the build are as follows:

| Tools      | version | link |
|-----------|---------|---------|
| CMake | 4.2.1 | https://cmake.org/download/ |
| clang/clang++ | 22.1.8 | https://github.com/llvm/llvm-project/releases/tag/llvmorg-22.1.8 |
| Ninja | 1.13.2 | https://github.com/ninja-build/ninja/releases/tag/v1.13.2 |

### Build Command

1. Configure & Generate Build Files  
Note that tests is only available in CI pipeline.

```cmd
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBUILD_TESTING=OFF
```

2. Build / Link the Project

```cmd
cmake --build build
```

## Documentation
- [Folder structure](docs/folder_structure.md)
- [Development notes](docs/Car_Simulator_Dev_Notes.md)
- [Class architecture](docs/class_architecture.md)
- [Class diagram](docs/class_diagram.md)
- [Build system architecture](docs/build_system_architecture.md)
- [CI process](docs/CI_Process.md)


## Development Plan
### Simulation environment
- [ ] Introduce reinforcement learning for the parking
    - [ ] Research RL libraries for C++
    - [X] Build an environment like gymnasium-style environment in Python
    - [ ] Introduce continuous action space
    - [ ] Implement RL
    - [ ] Training
    - [ ] Evaluation

### Future development ideas
- Path finding
- Decision making
- Reinforcement learning
- Sensors
- 3D environment


## Reference

[Draw 2D Shapes C++ OpenGL from Scratch](https://www.youtube.com/watch?v=OI-6aYTWl4w)  
[OpenGL 入門](http://www.center.nitech.ac.jp/~kenji/Study/Lib/ogl/)  
[Hello Triangle](https://learnopengl.com/Getting-started/Hello-Triangle)  
https://tokoik.github.io/GLFWdraft.pdf
https://zenn.dev/nyanchu_program/articles/97637278839801
https://codelabo.com/posts/20200228150223
