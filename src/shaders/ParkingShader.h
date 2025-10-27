#ifndef PARKING_H
#define PARKING_H

#include "ShaderProgram.h"


extern ShaderPaths PARKING_SHADER_PATHS;


class ParkingShader : public ShaderProgram {
public:
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    ParkingShader();
    

    // Not changing behavior; inherit the base use()
    using ShaderProgram::use;

};


#endif
