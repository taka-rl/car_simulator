#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;


// paths for vertex and fragment shaders
// ----------------------------------------------------------------------------
struct ShaderPaths {
    const string vertexPath;
    const string fragmentPath;
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
    ~ShaderProgram() { if (ID) glDeleteProgram(ID); }

    // activate the shader
    // ------------------------------------------------------------------------
    void use();

    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const string &name, bool value) const;

    // ------------------------------------------------------------------------
    void setInt(const string &name, int value) const;

    // ------------------------------------------------------------------------
    void setFloat(const string &name, float value) const;

    // getter for shader ID
    // ------------------------------------------------------------------------
    const unsigned int getShaderID();

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, const string type);

    // loads shader source code from files
    // ------------------------------------------------------------------------
    int loadShaderSource(unsigned int shader, string path);

    // creates shader program from vertex and fragment shader paths
    // ------------------------------------------------------------------------
    int makeShader(ShaderPaths paths);
    
};
#endif
