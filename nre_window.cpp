#include "nre_window.hpp"

// std
#include <stdexcept>

namespace nre
{

    NreWindow::NreWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
    {
        initWindow();
    }

    void NreWindow::initWindow()
    {

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }

    void NreWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    NreWindow::~NreWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}