#include "Window.h"

#include<iostream>


bool Window::s_glfwInitialized = false;

// constructor
// ------------------------------------------------------------------------
Window::Window(int width, int height, const char* title) {

    // Initialize GLFW once
    if (!s_glfwInitialized) {
        if (!glfwInit()) {
            std::cerr << "Faild to initialize GLFW\n";
            return;
        }
        s_glfwInitialized = true;
    }

    // Configure OpenGL context
    // ------------------------------
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    m_window = glfwCreateWindow(width, height, "Car Simulator", NULL, NULL);
    if (m_window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // Turn on vsync 60FPS
    glfwSwapInterval(1);
}

// deconstructor
// ------------------------------------------------------------------------
Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    if (s_glfwInitialized) {
        glfwTerminate();
        s_glfwInitialized = false;
    }
}

