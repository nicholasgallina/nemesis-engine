#pragma once

#include "nre_camera.hpp"
#include "nre_pipeline.hpp"
#include "nre_device.hpp"
#include "nre_game_object.hpp"
#include "nre_frame_info.hpp"

// std
#include <memory>
#include <vector>

namespace nre
{
    class SimpleRenderSystem
    {

    public:
        SimpleRenderSystem(NreDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const NreWindow &) = delete;
        SimpleRenderSystem &operator=(const NreWindow &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, std::vector<NreGameObject> &gameObjects);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        NreDevice &nreDevice;
        std::unique_ptr<NrePipeline> nrePipeline;
        VkPipelineLayout pipelineLayout;
    };

}