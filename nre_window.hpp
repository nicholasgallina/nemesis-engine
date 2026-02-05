#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace nre
{

    class NreWindow
    {

    public:
        NreWindow(int w, int h, std::string name);
        ~NreWindow();

        NreWindow(const NreWindow &) = delete;
        NreWindow &operator=(const NreWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }

    private:
        void initWindow();

        const int width;
        const int height;

        std::string windowName;
        GLFWwindow *window;
    };

} // namespace nre