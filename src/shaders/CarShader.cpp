#include "CarShader.h"


ShaderPaths CAR_SHADER_PATHS = {"./src/shaders/carShader.vert", "./src/shaders/carShader.frag"};


// constructor generates the shader on the fly
// ------------------------------------------------------------------------
CarShader::CarShader() : ShaderProgram(CAR_SHADER_PATHS) {}
