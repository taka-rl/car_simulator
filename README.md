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
    │   ├── car.cpp                     # Car class
    │   ├── car.h                       #    
    │   ├── glad.c                      # 
    │   ├── main.cpp                    # 
    │   ├── main_car.cpp                # Temp cpp file for car.cpp
    │   └── opengl_learning.cpp         # Temp cpp file for learning OpenGL
    └── README.md                       # Project documentation

## Build setting

### Build command
```cmd
g++ src/main.cpp src/glad.c -o output/program_rectangle -Llib -Iinclude -lglfw3dll
g++ src/opengl_learning.cpp src/glad.c -o output/program_rectangle -Llib -Iinclude -lglfw3dll
g++ src/car.cpp src/main_car.cpp -o output/program_car.exe

```

## Development Plan
### OpenGL
#### 2D
- [ ] Learn OpenGL basics
draw a single rectangle  
[Draw 2D Shapes C++ OpenGL from Scratch](https://www.youtube.com/watch?v=OI-6aYTWl4w)  
[OpenGL 入門](http://www.center.nitech.ac.jp/~kenji/Study/Lib/ogl/)  
[Hello Triangle](https://learnopengl.com/Getting-started/Hello-Triangle)  

    - [ ] Draw a rectangle
    - [ ] Key inputs
    - [ ] Create a simple map
- [ ] Draw a car with each wheel
- [ ] Draw a car trajectory line
- [ ] Draw a trafic road
- [ ] Expand the map
- [ ] Create discrete/continuous modes

### Car
- [ ] Implement Kinematic bicycle model
- [ ] Implement Dynamic bicycle model

### Future development ideas
- 3D environment
- Path finding
- Decision making
- Reinforcement learning
- Sensors
