#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>


// simulation state
struct State { float x, y; };

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float& ix, float& iy);
void step(State& prevState, State& curState, const double& simDt, float& ix, float& iy);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

/*
vertexShaderSource defines vertex shader that needs to be written in GLSL which is similar to C.
In vertexShaderSource, you must declare all the input vertex attributes.
vec2 is a data type that represents variables for 2D coordinate.
vec3 is a data type that represents variables for 3D coordinate.
layout is used for the location of the input variable.
gl_Position is a predefined variable that store the position data
- Example code
#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
*/
/*
fragmentShaderSource defines fragment shader that calculates the color output of your pixels. 
Fragment shader only requires one output varialbe that contains a vector of size 4 that define the final color output.
vec4(1.0f, 0.5f, 0.2f, 1.0f) --> output orange-ish color.

- Example code
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 

*/

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform vec2 uOffset;\n"
    "void main()\n"
    "{\n"
    "   vec3 p = aPos + vec3(uOffset, 0.0);\n"
    "   gl_Position = vec4(p, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" // 1.0f, 0.8f, 0.2f, 1.0f: Yellow-ish color
    "}\n\0";



// Clamp accumulator to avoid spiral of death after stalls
inline void clampAccumulator(double& accum, const double simDt, double maxSteps = 5.0) {
    const double MAX_ACCUM = simDt * maxSteps;
    if (accum > MAX_ACCUM) accum = MAX_ACCUM;
}

// Linear interpolation for positions
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Interpolate state (positions); for headings, use lerpAngle on psi
inline State interp(const State& prev, const State& curr, float alpha) {
    return State{
        lerp(prev.x, curr.x, alpha),
        lerp(prev.y, curr.y, alpha)
    };
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices1[] = { -1,-0.5,  1,-0.5,  1,0.5,  -1,-0.5,  1,0.5, -1,0.5 };
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    /*
    TODO: Deepen the understanding of VBO, VAO and EBO later.
    - VBO
    VBO, which stands for Vertex Buffer Objects, can store a large number of vertices in the GPU's memory. 
    A vertex buffer object has a unique ID corresponding to that buffer. The unique ID can be generated by `glGenBuffers` function.
    `glBindBuffer` function is used to bind the newly created buffer to the `GL_ARRAY_BUFFER` target. --> glBindBuffer(GL_ARRAY_BUFFER, VBO);
    `glBufferData` function is used to copy user-defined data into the currently bound buffer.  --> glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    - VAO
    VAO, which stands for Vertex Array Objects, can be bound just like a vertex buffer object.
    This has the advantage that when configuring vertex attribute pointers you only have to make those calls once and whenever we want to draw the object, 
    we can just bind the corresponding VAO. This makes switching between different vertex data and attribute configurations as easy as binding a different VAO. 

    A vertex array object can be stored in the following steps.
    1. Calls to glEnableVertexAttribArray or glDisableVertexAttribArray.
    2. Vertex attribute configurations via glVertexAttribPointer.
    3. Vertex buffer objects associated with vertex attributes by calls to glVertexAttribPointer.

    - EBO
    EBO, which stands for Element Buffer Objects, is a buffer same as a vertex buffer object.
        glGenBuffers(1, &EBO);
    A element buffer object can be stored in the following steps.
    1. Define the element buffer object --> glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    2. Similar to the VBO, the EBO needs to be bound and the indices are copied into the buffer with `glBindBuffer` and `glBufferData` functions.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    `glDrawElements` function takes its indices from the EBO bound to the GL_ELEMENT_ARRAY_BUFFER target. 
    This means we have to bind the corresponding EBO each time we want to render an object with indices which again is a bit cumbersome. 
    It just so happens that a vertex array object also keeps track of element buffer object bindings. 
    The last element buffer object that gets bound while a VAO is bound, is stored as the VAO's element buffer object. 
    Binding to a VAO then also automatically binds that EBO.
    
    */
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    // 1. bind vertex array object
    glBindVertexArray(VAO);

    // 2. copy our vertices array in a vertex buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. copy our index array in a element buffer for OpenGL to use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 4. then set the  vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // grab uniform location once
    glUseProgram(shaderProgram);
    int uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset");

    // Turn on vsync 60FPS
    glfwSwapInterval(1);

    // Simulation Config
    const double simDt = 0.01;
    double accumulator = 0.0;
    double lastTime = glfwGetTime();

    // simulation state (previous and current for interpolation)
    State prevState{0,0}, curState{0,0};

    // inputs
    float ix = 0.0f, iy = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // timing
        double now = glfwGetTime();
        double frameDt = now - lastTime;
        lastTime = now;
        accumulator += frameDt;

        // input
        // -----
        processInput(window, ix, iy);
    
        clampAccumulator(accumulator, simDt);

        // fixed-step simulation
        while (accumulator >= simDt) {
            step(prevState, curState, simDt, ix, iy);
            accumulator -= simDt;
        }

        // interpolate for smooth rendering
        float alpha = static_cast<float>(accumulator / simDt);
        State drawS = interp(prevState, curState, alpha);
        glUniform2f(uOffsetLoc, drawS.x, drawS.y);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw a rectangle
        // draw our first triangle
        glUseProgram(shaderProgram);
        glUniform2f(uOffsetLoc, drawS.x, drawS.y);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, float& ix, float& iy)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Move a rectangle based on inputs
    float dx = 0.0f, dy = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) dx += 1.0;
    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) dx -= 1.0;
    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) dy += 1.0;
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) dy -= 1.0;

    // simple critically-damped-ish smoothing toward target inputs (optional)
    // makes input changes less jittery between frames
    const float k = 0.25f; // smoothing factor [0..1], 0=no change, 1=instant
    ix = ix + k * (dx - ix);
    iy = iy + k * (dy - iy);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// simulation step
// ---------------------------------------------------------------------------------------------------------
void step(State& prevState, State& curState, const double& simDt, float& ix, float& iy) {
    // simple kinematic “speed” in NDC units per second
    const float SPEED = 0.8f;

    // shift previous → current for interpolation
    prevState = curState;

    // apply input as velocity command
    curState.x += (ix * SPEED) * static_cast<float>(simDt);
    curState.y += (iy * SPEED) * static_cast<float>(simDt);

    // keep quad fully on screen: NDC [-1, +1], quad half-size = 0.5
    const float margin = 0.5f;
    curState.x = std::clamp(curState.x, -1.0f + margin, 1.0f - margin);
    curState.y = std::clamp(curState.y, -1.0f + margin, 1.0f - margin);
};

