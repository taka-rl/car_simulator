
cmake_minimum_required(VERSION 3.14)
project(CarSimulator VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source root
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add source files
set(SOURCES
    ${SRC_DIR}/shaders/ShaderProgram.cpp
    ${SRC_DIR}/shaders/RectShader.cpp
    ${SRC_DIR}/entities/Entity.cpp
    ${SRC_DIR}/renderers/Renderer.cpp
    ${SRC_DIR}/utilities/Randomizer.cpp    
    ${SRC_DIR}/vehicledynamics/BicycleModel.cpp
    ${SRC_DIR}/envs/ParkingEnv.cpp
    ${SRC_DIR}/simulator/Simulator.cpp
    ${SRC_DIR}/Loader.cpp
    ${SRC_DIR}/Window.cpp
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/glad.c
)

# Create executable
add_executable(CarSimulator ${SOURCES})

# Core library (NO OpenGL / NO Window / NO main) for CI tests
add_library(car_core
  ${SRC_DIR}/envs/ParkingEnv.cpp
  ${SRC_DIR}/vehicledynamics/BicycleModel.cpp
  ${SRC_DIR}/utilities/Randomizer.cpp
)

# Headers (your glad/GLFW headers live in include/)
target_include_directories(CarSimulator PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# Headers for CI tests
target_include_directories(car_core PUBLIC ${PROJECT_SOURCE_DIR}/src)

# Library and include
if (WIN32)
  # --- Windows path (your current setup) ---
  # Link GLFW import lib directly + system libs
  target_link_libraries(CarSimulator PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/libglfw3dll.a   # <— your import lib
    opengl32
    gdi32
    user32
    shell32
  )

  # local includes/libraries will be removed later so GLFW will be needed to install on Windows (e.g. vcpkg install glfw3)
  # find_package(OpenGL REQUIRED)

  # Depending on how GLFW is installed, the package name may be glfw or glfw3.
  # You might need to adjust this line based on your system.
  # find_package(glfw3 REQUIRED)

elseif(APPLE)
  # --- macOS path ---
  # local includes/libraries will be removed later so GLFW will be needed to install on macOS (e.g. Homebrew: brew install glfw)
  # find_package(OpenGL REQUIRED)

  # Depending on how GLFW is installed, the package name may be glfw or glfw3.
  # You might need to adjust this line based on your system.
  # find_package(glfw3 REQUIRED)

  # target_link_libraries(CarSimulator PRIVATE
  #     glfw                          # or glfw3, depending on the package
  #     OpenGL::GL
  # )  

else()
  # --- Linux or others (future) ---
endif()

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
  target_link_libraries(${TEST_NAME} PRIVATE car_core GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(${TEST_NAME})
endif()
