#ifndef RECTSHADER_H
#define RECTSHADER_H

#include "ShaderProgram.h"


extern ShaderPaths RECT_SHADER_PATHS;


class RectShader : public ShaderProgram {

private:
    int uOffsetLoc_ = -1;
    int uColorLoc_ = -1;

public:
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    RectShader();

    // getter
    // ------------------------------------------------------------------------
    const int getUOffsetLoc() const noexcept;
    const int getUColor() const noexcept;

    // setter
    // ------------------------------------------------------------------------
    void setUniformLocation();
    void setUniformColor();

    void setColor(float r, float g, float b, float a) const;

    void setOffset(float x, float y) const;
    

    // Not changing behavior; inherit the base use()
    using ShaderProgram::use;

};
#endif
