#include "nre_window.hpp"

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

    NreWindow::~NreWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}