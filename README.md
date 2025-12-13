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
OS Windows 10

### Library
| Library      | version | link |
|-----------|---------|---------| 
| GLFW    | 3.4 | https://www.glfw.org/download.html |
| GLAD | Refer to https://rpxomi.github.io/  | https://glad.dav1d.de/ |
| C++ g++ compiler (Windows 10)| 13.1.0   | - |

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


## Build setting
### Build command
- without CMake
```cmd
g++ -std=c++17 src/glad.c src/main.cpp src/Window.cpp src/Loader.cpp src/shaders/ShaderProgram.cpp src/shaders/RectShader.cpp src/entities/Entity.cpp src/renderers/Renderer.cpp src/vehicledynamics/BicycleModel.cpp src/utilities/Randomizer.cpp src/simulator/Simulator.cpp -o output/program -Llib -Iinclude -lglfw3dll
```
- CMake
1. Configure & Generate Build Files
```
cmake -B build -S . -DBUILD_TESTING=OFF (Without tests)
```
or
``` 
cmake -B build -S . -DBUILD_TESTING=ON (With tests)
```

2. Build / Link the Project
```
cmake --build build --config Release
```

## Documentation
- [Folder structure](docs/folder_structure.md)
- [Development notes](docs/Car_Simulator_Dev_Notes.md)
- [Class architecture](docs/class_architecture.md): will be uploaded later
- [Class diagram](docs/class_diagram.md)
- [CI process](docs/CI_Process.md)


## Development Plan
### Simulation environment
- [ ] Introduce reinforcement learning for the parking
    - [ ] Research RL libraries for C++
    - [ ] Build an environment like gymnasium-style environment in Python
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
