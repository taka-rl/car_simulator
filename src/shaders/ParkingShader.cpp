#include "ParkingShader.h"


ShaderPaths PARKING_SHADER_PATHS = {"./src/shaders/parkingShader.vert", "./src/shaders/parkingShader.frag"};


// constructor generates the shader on the fly
// ------------------------------------------------------------------------
ParkingShader::ParkingShader() : ShaderProgram(PARKING_SHADER_PATHS) {}

