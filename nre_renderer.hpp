#pragma once

#include "nre_window.hpp"
#include "nre_device.hpp"
#include "nre_swap_chain.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace nre
{
    class NreRenderer
    {

    public:
        NreRenderer(NreWindow &window, NreDevice &device);
        ~NreRenderer();

        NreRenderer(const NreWindow &) = delete;
        NreRenderer &operator=(const NreWindow &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return nreSwapChain->getRenderPass(); }

        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapchain();

        NreWindow &nreWindow;
        NreDevice &nreDevice;
        std::unique_ptr<NreSwapChain> nreSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex;
        bool isFrameStarted;
    };

}