#include "first_app.hpp"

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

    struct SimplePushConstantData
    {
        // identity matrix
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    FirstApp::FirstApp()
    {
        loadGameObjects();
        createPipelineLayout();
        recreateSwapchain();
        createCommandBuffers();
    }

    FirstApp::~FirstApp()
    {
        vkDestroyPipelineLayout(nreDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::run()
    {
        while (!NreWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
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

    void FirstApp::createPipelineLayout()
    {

        // want access to push constant data in both shaders
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(nreDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    };

    void FirstApp::createPipeline()
    {
        assert(nreSwapChain != nullptr && "Cannot create pipeline before SwapChain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before Pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        NrePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = nreSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        nrePipeline = std::make_unique<NrePipeline>(
            nreDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void FirstApp::recreateSwapchain()
    {
        auto extent = NreWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = NreWindow.getExtent();
            glfwWaitEvents();
        }

        // wait until current SwapChain is no longer being used
        // until new SwapChain is created
        vkDeviceWaitIdle(nreDevice.device());

        if (nreSwapChain == nullptr)
        {
            nreSwapChain = std::make_unique<NreSwapChain>(nreDevice, extent);
        }
        else
        {
            nreSwapChain = std::make_unique<NreSwapChain>(nreDevice, extent, std::move(nreSwapChain));
            if (nreSwapChain->imageCount() != commandBuffers.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            };
        };

        createPipeline();
    }

    void FirstApp::createCommandBuffers()
    {

        commandBuffers.resize(nreSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = nreDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(nreDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers");
        }
    }

    void FirstApp::freeCommandBuffers()
    {
        vkFreeCommandBuffers(nreDevice.device(), nreDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());
        commandBuffers.clear();
    }

    void FirstApp::recordCommandBuffer(int imageIndex)
    {

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = nreSwapChain->getRenderPass();
        renderPassInfo.framebuffer = nreSwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = nreSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(nreSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(nreSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, nreSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        renderGameObjects(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer");
        }
    }

    void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        nrePipeline->bind(commandBuffer);

        for (auto &obj : gameObjects)
        {
            obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }

    // detect here whether SwapChain has been resized and decide whether it needs
    //  to be recreated
    void FirstApp::drawFrame()
    {
        uint32_t imageIndex;
        auto result = nreSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapchain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to aqcuire swapChain image");
        }

        recordCommandBuffer(imageIndex);
        result = nreSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || NreWindow.wasWindowResized())
        {
            NreWindow.resetWindowResizedFlag();
            recreateSwapchain();
            return;
        }
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swapChain image");
        }
    }
} // namspace nre