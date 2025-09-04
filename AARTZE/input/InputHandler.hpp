#pragma once
#include <GLFW/glfw3.h>

// Minimal input handler: captures basic window events and ESC to close.
class InputHandler
{
public:
    void SetWindow(GLFWwindow* window) { m_window = window; }
    void Poll();

private:
    GLFWwindow* m_window{nullptr};
};
