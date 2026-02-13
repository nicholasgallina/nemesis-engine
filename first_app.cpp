#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "nre_camera.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS // no matter the system, GLM expects radians
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <chrono>

namespace nre
{

    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp()
    {
    }

    // game loop
    void FirstApp::run()
    {
        SimpleRenderSystem SimpleRenderSystem{nreDevice, nreRenderer.getSwapChainRenderPass()};
        NreCamera camera{};
        // camera.setViewDirection(glm::vec3(0.f), glm::vec3(1.f, 0.f, 1.f));
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

        // doesn't have a model
        // won't be rendered
        // used to store camera's current state
        auto viewerObject = NreGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!nreWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.moveInPlaneXZ(nreWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = nreRenderer.getAspectRatio();
            // placed inside so it's updated when the aspect ratio changes
            // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

            // beginFrame returns a null ptr if SwapChain needs to be recreated
            // beginFrame and beginSwapChainRenderPass are separate functions
            // to retain control for future extendability
            // ie: multiple renderPass(es) for reflection, shadow, post-processing
            if (auto commandBuffer = nreRenderer.beginFrame())
            {
                nreRenderer.beginSwapChainRenderPass(commandBuffer);
                SimpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
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
        cube.transform.translation = {.0, .0f, 2.5f};
        cube.transform.scale = {.5f, .5f, .5f};
        gameObjects.push_back(std::move(cube));
    };
} // namspace nre