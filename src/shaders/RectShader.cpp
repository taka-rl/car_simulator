#include "RectShader.h"


ShaderPaths RECT_SHADER_PATHS = {"./src/shaders/rectShader.vert", "./src/shaders/rectShader.frag"};


// constructor generates the shader on the fly
// ------------------------------------------------------------------------
RectShader::RectShader() : ShaderProgram(RECT_SHADER_PATHS) {
    cacheOffsetLocation();
    cacheColorLocation();
    cacheScaleLocation();
}

// getter
// ------------------------------------------------------------------------
const int RectShader::getUOffsetLoc() const noexcept { return uOffsetLoc_; };
const int RectShader::getUColor() const noexcept { return uColorLoc_; };

// setter 
// set color
// ------------------------------------------------------------------------
void RectShader::setColor(float r, float g, float b, float a) const {
    if (uColorLoc_ != -1) glUniform4f(uColorLoc_, r, g, b, a);
}

// set location
// ------------------------------------------------------------------------
void RectShader::setOffset(float x, float y) const {
    if (uOffsetLoc_ != -1) glUniform2f(uOffsetLoc_, x, y);
}

void RectShader::setScale(float x, float y) const {
    if (uScaleLoc_ != -1) glUniform2f(uScaleLoc_, x, y);
}

// cache uOffset
// ------------------------------------------------------------------------
void RectShader::cacheOffsetLocation() {
    uOffsetLoc_ = glGetUniformLocation(ID, "uOffset");
}

// cache uColor
// ------------------------------------------------------------------------
void RectShader::cacheColorLocation() {
    uColorLoc_ = glGetUniformLocation(ID, "uColor");
}

// cache uScale
// ------------------------------------------------------------------------
void RectShader::cacheScaleLocation() {
    uScaleLoc_ = glGetUniformLocation(ID, "uScale");
}