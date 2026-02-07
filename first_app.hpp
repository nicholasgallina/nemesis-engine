#pragma once

#include "nre_window.hpp"
#include "nre_pipeline.hpp"
#include "nre_device.hpp"
#include "nre_swap_chain.hpp"
#include "nre_model.hpp"

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
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        NreWindow NreWindow{WIDTH, HEIGHT, "Nebula Rendering Engine"};
        NreDevice nreDevice{NreWindow};
        NreSwapChain nreSwapChain{nreDevice, NreWindow.getExtent()};
        std::unique_ptr<NrePipeline> nrePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<NreModel> nreModel;
    };

}