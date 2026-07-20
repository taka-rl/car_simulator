# Car Simulator - CI & Testing overview

This document describes how the CI pipeline works for the **Car Simulator** project.

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
include(FetchContent)

# ------------------------------------------------------------------------------
# GLFW
# ------------------------------------------------------------------------------

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw)

# ------------------------------------------------------------------------------
# Core simulation logic
# ------------------------------------------------------------------------------
add_library(car_core STATIC
  ${SRC_DIR}/vehicledynamics/BicycleModel.cpp
  ${SRC_DIR}/utilities/Randomizer.cpp
)

target_include_directories(car_core
    PUBLIC
        ${SRC_DIR}
)

# ------------------------------------------------------------------------------
# Rendering and window system
# ------------------------------------------------------------------------------
add_library(car_render STATIC
  ${SRC_DIR}/entities/Entity.cpp
  ${SRC_DIR}/renderers/Renderer.cpp
  ${SRC_DIR}/shaders/ShaderProgram.cpp
  ${SRC_DIR}/shaders/RectShader.cpp
  ${SRC_DIR}/Window.cpp
  ${SRC_DIR}/Loader.cpp
  ${SRC_DIR}/glad.c
)

target_include_directories(car_render
    PUBLIC
        ${SRC_DIR}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(car_render
    PUBLIC
        GLFW_INCLUDE_NONE
)

target_link_libraries(car_render
    PUBLIC
        car_core
        glfw
)

# ------------------------------------------------------------------------------
# Simulation environments
# ------------------------------------------------------------------------------
add_library(car_env STATIC
  ${SRC_DIR}/envs/ParkingEnv.cpp
)

target_include_directories(car_env
    PUBLIC
        ${SRC_DIR}
)

target_link_libraries(car_env
    PUBLIC
        car_core
)

# ------------------------------------------------------------------------------
# Final application
# ------------------------------------------------------------------------------
add_executable(CarSimulator
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/simulator/Simulator.cpp
)

# Headers (your glad/GLFW headers live in include/)
target_include_directories(CarSimulator PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# Internal dependencies are required on every platform.
target_link_libraries(CarSimulator
    PRIVATE
        car_core
        car_env
        car_render
)
```

### Tests (GoogleTest)

Tests are only enabled when BUILD_TESTING=ON:

```cmake
# Enable testing (optional)
include(CTest)
enable_testing()

if (BUILD_TESTING)
  # CI gtest setting 
  include(FetchContent)

  # Avoid CRT mismatch warnings on MSVC
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0   # C++17
  )
  FetchContent_MakeAvailable(googletest)

  set(TEST_NAME ${PROJECT_NAME}_tests)
  add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_parking_math.cpp)
  target_link_libraries(${TEST_NAME} PRIVATE car_env GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(${TEST_NAME})
endif()
```

## 3. GitHub Actions Workflow

The CI workflow lives in .github/workflows/ci.yml and currently:

- Runs on both Windows (windows-latest) and Mac (macos-latest).
- Configures CMake with BUILD_TESTING=ON.
- This is off temoprarily: Builds in Release mode. (Other modes will be added later.)
- Runs all tests via ctest.

## 4. Build

### 4.1 Build without tests (local dev, faster)

```
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBUILD_TESTING=OFF

cmake --build build
```

### 4.2 Build with tests

```
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBUILD_TESTING=ON

cmake --build build

ctest --output-on-failure
```

## 5. Adding new tests

1. Add a new TEST block in tests/test_parking_math.cpp
(or a new test file, and add it to CMake if you split things later).
Make sure that you follow the instruction of "### `CarSimulator_tests`" and "## Future Test Structure" in [build_system_architecture](docs/build_system_architecture)

2. Make sure your core math functions are accessible from tests
(preferably via a small header like parking_math.h).

3. Run locally with the command on 4. Build

4. Push your branch – CI will run the same sequence automatically.

## 6. Future development ideas

- Measure code coverage
- Add more test cases
