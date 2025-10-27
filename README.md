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
    |   │   ├── CarShader.cpp           # CarShader class 
    |   │   ├── CarShader.h             # CarShader class header
    |   │   ├── ParkingShader.cpp       # ParkingShader class 
    |   │   ├── ParkingShader.h         # ParkingShader class header
    |   │   ├── carShader.vert          # Shader for car
    |   │   ├── carShader.frag          # Shader for car
    |   │   ├── parkingShader.vert      # Shader for parking
    |   │   ├── parkingShader.frag      # Shader for parking
    |   │   ├── ShaderProgram.cpp       # ShaderProgram class 
    |   │   └── ShaderProgram.h         # ShaderProgram class header
    │   ├── car.cpp                     # Car class
    │   ├── car.h                       #    
    │   ├── glad.c                      # 
    │   ├── Loader.cpp                  # Loader class
    │   ├── Loader.h                    # Loader class header
    │   ├── main.cpp                    # 
    │   ├── main_car.cpp                # Temp cpp file for car.cpp
    ├── glfw3.dll                       # 
    └── README.md                       # Project documentation
    

## Build setting

### Build command
```cmd
g++ src/main.cpp src/glad.c -o output/program_rectangle -Llib -Iinclude -lglfw3dll
g++ -std=c++17 src/glad.c src/main.cpp  src/shaders/*.cpp  src/Loader.cpp -o output/program -Llib -Iinclude -lglfw3dll
g++ src/car.cpp src/main_car.cpp -o output/program_car.exe

```

## Development Plan
### OpenGL
#### 2D
- [X] Learn OpenGL basics
    - [X] Draw a rectangle
    - [X] Key inputs
    - [X] Move a rectangle based on the key inputs
    - [X] Understand basics of rendering on screen
    - [X] Create ShaderProgram class
    - [X] Load GLSL files from different files
    - [X] Draw two rectangles as a car and parking lot

### Simulation environment
- [ ] Develop a parking environment
    - [ ] Draw a car with each wheel
    - [ ] Draw a car trajectory line
    - [ ] Create a simple map
    - [ ] Introcude a kinematic bicycle model to simulate the car movement
    - [ ] Introduce acceleration and steering inputs with discrete action space
    - [ ] Introduce continuous action space
    - [ ] Set the parking lot randomly
    - [ ] Add code to determine if the car is parked
- [ ] Introduce reinforcement learning for the parking


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
