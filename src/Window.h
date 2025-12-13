#ifndef WINDOW_H
#define WINDOW_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>


class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    // return whether it's valid or not
    // ------------------------------------------------------------------------
    bool isValid() const { return m_window != nullptr; }

    // expose raw pointer so Simulator can keep using GLFW APIs
    // ------------------------------------------------------------------------
    GLFWwindow* get() const { return m_window; }

private:
    static bool s_glfwInitialized;
    GLFWwindow* m_window{nullptr};
};


#endif