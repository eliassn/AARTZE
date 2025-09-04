#include "InputHandler.hpp"
#include <GLFW/glfw3.h>

void InputHandler::Poll()
{
    if (!m_window) return;
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}
