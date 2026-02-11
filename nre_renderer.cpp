#include "nre_renderer.hpp"

// std
#include <stdexcept>
#include <array>

namespace nre
{

    NreRenderer::NreRenderer(NreWindow &window, NreDevice &device) : nreWindow{window}, nreDevice{device}, isFrameStarted{false}, currentImageIndex{0}
    {
        recreateSwapchain();
        createCommandBuffers();
    }

    NreRenderer::~NreRenderer()
    {
        freeCommandBuffers();
    }

    void NreRenderer::recreateSwapchain()
    {
        auto extent = nreWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = nreWindow.getExtent();
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
            std::shared_ptr<NreSwapChain> oldSwapChain = std::move(nreSwapChain);
            nreSwapChain = std::make_unique<NreSwapChain>(nreDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*nreSwapChain.get()))
            {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        };

        // tbd
    }

    void NreRenderer::createCommandBuffers()
    {

        commandBuffers.resize(NreSwapChain::MAX_FRAMES_IN_FLIGHT);

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

    void NreRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(nreDevice.device(), nreDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer NreRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");
        auto result = nreSwapChain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapchain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to aqcuire swapChain image");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording command buffer");
        }
        return commandBuffer;
    }
    void NreRenderer::endFrame()
    {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer");
        }

        auto result = nreSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || nreWindow.wasWindowResized())
        {
            nreWindow.resetWindowResizedFlag();
            recreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swapChain image");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % NreSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void NreRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call beginSwapChain if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin Render Pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = nreSwapChain->getRenderPass();
        renderPassInfo.framebuffer = nreSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = nreSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(nreSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(nreSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, nreSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    void NreRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call endSwapChain if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end Render Pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }
} // namspace nre