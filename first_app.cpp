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

    void sierpinksi(glm::vec2 a, glm::vec3 cA, glm::vec2 b, glm::vec3 cB, glm::vec2 c, glm::vec3 cC, int depth, std::vector<NreModel::Vertex> &vertices)
    {

        if (depth == 0)
        {
            vertices.push_back({a, cA});
            vertices.push_back({b, cB});
            vertices.push_back({c, cC});
        }
        else
        {
            glm::vec2 ab = (a + b) / 2.0f;
            glm::vec2 bc = (b + c) / 2.0f;
            glm::vec2 ca = (c + a) / 2.0f;

            glm::vec3 cAB = (cA + cB) / 2.0f;
            glm::vec3 cBC = (cB + cC) / 2.0f;
            glm::vec3 cCA = (cC + cA) / 2.0f;

            sierpinksi(a, cA, ab, cAB, ca, cCA, depth - 1, vertices);
            sierpinksi(b, cB, ab, cAB, bc, cBC, depth - 1, vertices);
            sierpinksi(c, cC, ca, cCA, bc, cBC, depth - 1, vertices);
        }
    }

    void sierpinksi(glm::vec2 a, glm::vec2 b, glm::vec2 c, int depth, std::vector<NreModel::Vertex> &vertices)
    {
        if (depth == 0)
        {
            vertices.push_back({a});
            vertices.push_back({b});
            vertices.push_back({c});
        }
        else
        {
            glm::vec2 ab = (a + b) / 2.0f;
            glm::vec2 bc = (b + c) / 2.0f;
            glm::vec2 ca = (c + a) / 2.0f;

            sierpinksi(a, ab, ca, depth - 1, vertices);
            sierpinksi(b, ab, bc, depth - 1, vertices);
            sierpinksi(c, ca, bc, depth - 1, vertices);
        };
    }

    void FirstApp::loadGameObjects()
    {
        std::vector<NreModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        auto nreModel = std::make_shared<NreModel>(nreDevice, vertices);

        auto triangle = NreGameObject::createGameObject();
        triangle.model = nreModel; // todo: check
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    };
} // namspace nre