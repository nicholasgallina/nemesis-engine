#pragma once

#include "nre_window.hpp"
#include "nre_pipeline.hpp"
#include "nre_device.hpp"
#include "nre_swap_chain.hpp"
#include "nre_game_object.hpp"

// std
#include <memory>
#include <vector>

namespace nre
{
    class FirstApp
    {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const NreWindow &) = delete;
        FirstApp &operator=(const NreWindow &) = delete;

        void run();

    private:
        void loadGameObjects();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapchain();
        void recordCommandBuffer(int imageIndex);
        void renderGameObjects(VkCommandBuffer commandBuffer);

        NreWindow NreWindow{WIDTH, HEIGHT, "Nebula Rendering Engine"};
        NreDevice nreDevice{NreWindow};
        std::unique_ptr<NreSwapChain> nreSwapChain;
        std::unique_ptr<NrePipeline> nrePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<NreGameObject> gameObjects;
    };

}