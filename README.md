# Car Simulator 

## Overview
Top-down 2D car simulator: **car body + 4 wheels**, meters-first physics with a **fixed timestep**, and smooth rendering via a single **unit-quad** mesh and a **RectShader** (scale → rotate → translate).


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
| Controls      | Description |
|-----------|---------| 
| Up | +acceleration |
| Down | -acceleration |
| Left | +steer(CCW) |
| Right | -steer(CW) |
| Escape | Quit |


## Documentation
- Folder structure 
Refer to [this page](https://github.com/taka-rl/car_simulator/docs/folder_structure.md)

- Development Note
Refer to [this page](https://github.com/taka-rl/car_simulator/docs/Car_Simulator_Dev_Notes.md)

## Build setting

### Build command
```cmd
g++ -std=c++17 src/glad.c src/main.cpp src/Loader.cpp src/shaders/ShaderProgram.cpp src/shaders/RectShader.cpp src/entities/Entity.cpp src/renderers/Renderer.cpp src/vehicledynamics/BicycleModel.cpp -o output/program -Llib -Iinclude -lglfw3dll

```

## Development Plan
### Simulation environment
- [ ] Develop a parking environment
    - [X] Draw a car with each wheel
    - [ ] Draw a car trajectory line
    - [ ] Create a simple map
    - [X] Introcude a kinematic bicycle model to simulate the car movement
    - [X] Introduce acceleration and steering inputs with discrete action space
    - [ ] Introduce continuous action space
    - [ ] Set the parking lot randomly
    - [ ] Add code to determine if the car is parked
- [ ] Introduce reinforcement learning for the parking
    - [ ] Research RL libraries for C++
    - [ ] Build an environment like gymnasium in Python
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
