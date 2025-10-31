#ifndef RECTSHADER_H
#define RECTSHADER_H

#include "ShaderProgram.h"


extern ShaderPaths RECT_SHADER_PATHS;


class RectShader : public ShaderProgram {

private:
    int uOffsetLoc_ = -1;
    int uColorLoc_ = -1;
    int uScaleLoc_ = -1;
    int uYawLoc_ = -1;

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
    void setColor(float r, float g, float b, float a) const;
    void setOffset(float x, float y) const;
    void setScale(float x, float y) const;
    void setYaw(float yaw) const;
    // Not changing behavior; inherit the base use()
    using ShaderProgram::use;

private:
    void cacheOffsetLocation();
    void cacheColorLocation();
    void cacheScaleLocation();
    void cacheYawLocation();

};
#endif
