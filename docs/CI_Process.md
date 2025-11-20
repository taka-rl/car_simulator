# Car Simulator - CI & Testing overview

This document describes how the CI pipeline works for the **Car Simulator** project. 

and how to run the same checks locally.


## Goals
- Ensure the project **builds** on a clean machine.
- Run **unit tests** (GoogleTest) for Car Simulator logic. 
- Keep the setup simple and **CMake-based**, so local builds and CI use the same commands.


### 1. Tools

- **CMake**: primary build system generator (Windows / macOS / Linux).
- **GoogleTest**: C++ unit test framework, fetched via `FetchContent` when `BUILD_TESTING=ON`.
- **GitHub Actions**: CI runner (currently `windows-latest`).

Key files:

- `CMakeLists.txt` – main build & test configuration.
- `tests/test_parking_math.cpp` – unit tests for parking math (`wrapPi`, `isParked`, etc.).
- `.github/workflows/ci.yml` – CI workflow definition.

## 2. CMake structure

### Executable

```cmake
set(SOURCES
    src/shaders/ShaderProgram.cpp
    src/shaders/RectShader.cpp
    src/entities/Entity.cpp
    src/renderers/Renderer.cpp
    src/vehicledynamics/BicycleModel.cpp
    src/Loader.cpp
    src/main.cpp
    src/glad.c
)

add_executable(CarSimulator ${SOURCES})
target_include_directories(CarSimulator PRIVATE ${CMAKE_SOURCE_DIR}/include)
```

On Windows, GLFW is linked using the local import lib in lib/:

```
if (WIN32)
  target_link_libraries(CarSimulator PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/libglfw3dll.a
    opengl32 gdi32 user32 shell32
  )
endif()
```

### Tests (GoogleTest)
Tests are only enabled when BUILD_TESTING=ON:

```cmake
include(CTest)
enable_testing()

if (BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  set(TEST_NAME ${PROJECT_NAME}_tests)
  add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_parking_math.cpp)
  target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(${TEST_NAME})
endif()
```

## 3. GitHub Actions Workflow

The CI workflow lives in .github/workflows/ci.yml and currently:

- Runs on Windows (windows-latest). (ther OSs will be added later.)
- Configures CMake with BUILD_TESTING=ON.
- Builds in Release mode. (Other modes will be added later.)
- Runs all tests via ctest.

## 4. Build
### 4.1 Build without tests (local dev, faster)
```
cmake -B build -S . -DBUILD_TESTING=OFF
cmake --build build --config Release
```

### 4.2 Build with tests
```
cmake -B build -S . -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## 5. Adding new tests

1. Add a new TEST block in tests/test_parking_math.cpp
(or a new test file, and add it to CMake if you split things later).

2. Make sure your core math functions are accessible from tests
(preferably via a small header like parking_math.h).

3. Run locally:
```
cmake -B build -S . -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

4. Push your branch – CI will run the same sequence automatically.

## 6. Future development ideas



