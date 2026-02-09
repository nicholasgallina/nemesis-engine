#include "first_app.hpp"

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
        loadModels();
        createPipelineLayout();
        createPipeline();
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

    void FirstApp::loadModels()
    {
        std::vector<NreModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        nreModel = std::make_unique<NreModel>(nreDevice, vertices);
    }

    void FirstApp::createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(nreDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void FirstApp::createPipeline()
    {
        auto pipelineConfig = NrePipeline::defaultPipelineConfigInfo(nreSwapChain.width(), nreSwapChain.height());
        pipelineConfig.renderPass = nreSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        nrePipeline = std::make_unique<NrePipeline>(
            nreDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void FirstApp::createCommandBuffers()
    {

        commandBuffers.resize(nreSwapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = nreDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(nreDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers");
        }

        for (int i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = nreSwapChain.getRenderPass();
            renderPassInfo.framebuffer = nreSwapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = nreSwapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            nrePipeline->bind(commandBuffers[i]);
            nreModel->bind(commandBuffers[i]);
            nreModel->draw(commandBuffers[i]);

            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to record command buffer");
            }
        }
    }
    void FirstApp::drawFrame()
    {
        uint32_t imageIndex;
        auto result = nreSwapChain.acquireNextImage(&imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to aqcuire swapChain image");
        }

        result = nreSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swapChain image");
        }
    }
} // namspace nre