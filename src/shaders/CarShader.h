#ifndef CARSHADER_H
#define CARSHADER_H

#include "ShaderProgram.h"


extern ShaderPaths CAR_SHADER_PATHS;


class CarShader : public ShaderProgram {
public:
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    CarShader();
    

    // Not changing behavior; inherit the base use()
    using ShaderProgram::use;

};
#endif
