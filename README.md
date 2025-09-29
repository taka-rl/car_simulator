# Car Simulator 
## Environment
OS Windows 10

### Library
| Library      | version | link |
|-----------|---------|---------| 
| GLFW    | 3.4 | https://www.glfw.org/download.html |
| GLAD | Refer to https://rpxomi.github.io/  | https://glad.dav1d.de/ |
| C++ g++ compiler (Windows 10)| 13.1.0   | - |

## Folder structure

    ├── include                         # include
    │   ├── glad                        # 
    |   │   └── glad.h                  # 
    │   ├── GLFW                        # 
    |   │   ├── glfw3.h                 # 
    |   │   └── glfw3native.h           # 
    │   ├── KHR                         # 
    |   │   └── khrplatform.h           # 
    ├── lib                             # library
    │   └── libglfw3dll.a               # 
    ├── src                             # src
    │   ├── shaders                     # 
    |   │   ├── carShader.vert          # 
    |   │   ├── carShader.frag          # 
    |   │   ├── ShaderProgram.cpp       # ShaderProgram class 
    |   │   └── ShaderProgram.h         # ShaderProgram class header
    │   ├── car.cpp                     # Car class
    │   ├── car.h                       #    
    │   ├── glad.c                      # 
    │   ├── main.cpp                    # 
    │   ├── main_car.cpp                # Temp cpp file for car.cpp
    │   └── opengl_learning.cpp         # Temp cpp file for learning OpenGL
    ├── glfw3.dll                       # 
    └── README.md                       # Project documentation
    

## Build setting

### Build command
```cmd
g++ src/main.cpp src/glad.c -o output/program_rectangle -Llib -Iinclude -lglfw3dll
g++ -std=c++17 src/glad.c src/opengl_learning.cpp  src/shaders/ShaderProgram.cpp -o output/program_opengl_learning -Llib -Iinclude -lglfw3dll
g++ src/car.cpp src/main_car.cpp -o output/program_car.exe

```

## Development Plan
### OpenGL
#### 2D
- [ ] Learn OpenGL basics
    - [X] Draw a rectangle
    - [X] Key inputs
    - [X] Move a rectangle based on the key inputs
    - [X] Understand basics of rendering on screen
    - [X] Create ShaderProgram class
    - [X] Load GLSL files from different files
    - [ ] Collision check

### Simulation environment
- [ ] Draw a car with each wheel
- [ ] Draw a car trajectory line
- [ ] Create a simple map
- [ ] Simulation setup
    - [ ] Introduce acceleration and steering inputs
    - [ ] Create discrete/continuous modes

### Car
- [ ] Implement Kinematic bicycle model
- [ ] Implement Dynamic bicycle model

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
