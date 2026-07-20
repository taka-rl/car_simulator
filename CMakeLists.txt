
cmake_minimum_required(VERSION 3.14)
project(CarSimulator VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source root
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

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

# Platform-specific dependencies
# if (WIN32)
#   # --- Windows path (your current setup) ---
#   # Link GLFW import lib directly + system libs
#   target_link_libraries(CarSimulator PRIVATE
#     opengl32
#   )
# 
#   # local includes/libraries will be removed later so GLFW will be needed to install on Windows (e.g. vcpkg install glfw3)
#   # find_package(OpenGL REQUIRED)
# 
#   # Depending on how GLFW is installed, the package name may be glfw or glfw3.
#   # You might need to adjust this line based on your system.
#   # find_package(glfw3 REQUIRED)
# 
# elseif(APPLE)
#   # --- macOS path ---
#   # local includes/libraries will be removed later so GLFW will be needed to install on macOS (e.g. Homebrew: brew install glfw)
#   # find_package(OpenGL REQUIRED)
# 
#   # Depending on how GLFW is installed, the package name may be glfw or glfw3.
#   # You might need to adjust this line based on your system.
#   # find_package(glfw3 REQUIRED)
# 
#   # target_link_libraries(CarSimulator PRIVATE
#   #     glfw                          # or glfw3, depending on the package
#   #     OpenGL::GL
#   # )  
# 
# else()
#   # --- Linux or others (future) ---
# endif()

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
