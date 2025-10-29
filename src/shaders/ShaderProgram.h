#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


// paths for vertex and fragment shaders
// ----------------------------------------------------------------------------
struct ShaderPaths {
    const std::string vertexPath;
    const std::string fragmentPath;
};


// a general shader program class that can be used for different shaders
// ----------------------------------------------------------------------------
class ShaderProgram
{
protected:
    unsigned int ID;

public:
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    ShaderProgram(ShaderPaths paths);

    // destructor 
    ~ShaderProgram();

    // activate the shader
    // ------------------------------------------------------------------------
    void use() const;

    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const;

    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const;

    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const;

    // getter
    // ------------------------------------------------------------------------
    const unsigned int getShaderID() const noexcept;

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, const std::string type);

    // loads shader source code from files
    // ------------------------------------------------------------------------
    int loadShaderSource(unsigned int shader, std::string path);

    // creates shader program from vertex and fragment shader paths
    // ------------------------------------------------------------------------
    int makeShader(ShaderPaths paths);
    
};
#endif
