#include "first_app.hpp"

#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS // no matter the system, GLM expects radians
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>

namespace colors
{
    const glm::vec3 RED = {1.0f, 0.0f, 0.0f};
    const glm::vec3 GREEN = {0.0f, 1.0f, 0.0f};
    const glm::vec3 BLUE = {0.0f, 0.0f, 1.0f};
    const glm::vec3 YELLOW = {1.0f, 1.0f, 1.0f};
    const glm::vec3 PURPLE = {0.5f, 0.0f, 0.5f};
}

namespace nre
{

    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp()
    {
    }

    void FirstApp::run()
    {
        SimpleRenderSystem SimpleRenderSystem{nreDevice, nreRenderer.getSwapChainRenderPass()};
        while (!nreWindow.shouldClose())
        {
            glfwPollEvents();

            // beginFrame returns a null ptr if SwapChain needs to be recreated
            // beginFrame and beginSwapChainRenderPass are separate functions
            // to retain control for future extendability
            // ie: multiple renderPass(es) for reflection, shadow, post-processing
            if (auto commandBuffer = nreRenderer.beginFrame())
            {
                nreRenderer.beginSwapChainRenderPass(commandBuffer);
                SimpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                nreRenderer.endSwapChainRenderPass(commandBuffer);
                nreRenderer.endFrame();
            }
        }

        // CPU will block until all GPU operations have been completed
        vkDeviceWaitIdle(nreDevice.device());
    }

    // temporary helper function, creates a 1x1x1 cube centered at offset
    std::unique_ptr<NreModel> createCubeModel(NreDevice &device, glm::vec3 offset)
    {
        std::vector<NreModel::Vertex> vertices{

            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

        };
        for (auto &v : vertices)
        {
            v.position += offset;
        }
        return std::make_unique<NreModel>(device, vertices);
    }

    void FirstApp::loadGameObjects()
    {
        std::shared_ptr<NreModel> nreModel = createCubeModel(nreDevice, {.0f, .0f, .0f});
        auto cube = NreGameObject::createGameObject();
        cube.model = nreModel;
        cube.transform.translation = {.0, .0f, .5f};
        cube.transform.scale = {.5f, .5f, .5f};
        gameObjects.push_back(std::move(cube));
    };
} // namspace nre