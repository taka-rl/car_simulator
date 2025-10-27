#include "ShaderProgram.h"


// constructor generates the shader on the fly
// ------------------------------------------------------------------------
ShaderProgram::ShaderProgram(ShaderPaths paths) { 
    ID = makeShader(paths);
    setUniFormLocation(ID, "uOffset");
};

// destructor
// ------------------------------------------------------------------------
ShaderProgram::~ShaderProgram() { 
    glDeleteProgram(ID);
    std::cout << "ShaderProgram destructed, shader program deleted." << std::endl;
};

// activate the shader
// ------------------------------------------------------------------------
void ShaderProgram::use() const { glUseProgram(ID); };

// utility uniform functions
// ------------------------------------------------------------------------
void ShaderProgram::setBool(const std::string &name, bool value) const {         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
};

// ------------------------------------------------------------------------
void ShaderProgram::setInt(const std::string &name, int value) const { 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
};

// ------------------------------------------------------------------------
void ShaderProgram::setFloat(const std::string &name, float value) const { 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
};

// getter for shader ID
// ------------------------------------------------------------------------
const unsigned int ShaderProgram::getShaderID() const noexcept { return ID; };
const unsigned int ShaderProgram::getUOffsetLoc() const noexcept { return uOffsetLoc; };

// setter 
void ShaderProgram::setUniFormLocation(unsigned int& ID, const std::string& name) {
    uOffsetLoc = glGetUniformLocation(ID, name.c_str());
};


// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void ShaderProgram::checkCompileErrors(unsigned int shader, const std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
};

int ShaderProgram::loadShaderSource(unsigned int shaderObj, const std::string path) {
    
    // TODO: Add code to adjust the path in any cases where .exe file is executed. 
    // Open the file
    std::ifstream ifs(path);
    if (!ifs) {
        std::cout << "error" << std::endl;
        return -1;
    }

    std::string source;
    std::string line;
    while (getline(ifs, line)) {
        source += line + "\n";
    }

    // Load shader code to shader object
    const char* sourcePtr = (const GLchar *)source.c_str();
    int length = source.length();
    glShaderSource(shaderObj, 1, &sourcePtr, &length);

    return 0;
};

int ShaderProgram::makeShader(ShaderPaths paths) {
    
    // Create the shader objects
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int shader;

    // Varables for compile and link
    // int compiled, linked;

    // Load shader source code from files
    if (loadShaderSource(vertex, paths.vertexPath)) return -1;
    if (loadShaderSource(fragment, paths.fragmentPath)) return -1;

    // Compile the vertex shader
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Compile the fragment shader
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // Link shaders
    shader = glCreateProgram();

    // Register the shaders
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);

    // Link the program
    glLinkProgram(shader);
    checkCompileErrors(shader, "PROGRAM");

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shader;
};
