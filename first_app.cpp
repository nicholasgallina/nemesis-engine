#include "first_app.hpp"

namespace nre
{

    void FirstApp::run()
    {
        while (!NreWindow.shouldClose())
        {
            glfwPollEvents();
        }
    }
} // namspace nre