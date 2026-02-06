#pragma once

#include "nre_window.hpp"
#include "nre_pipeline.hpp"
#include "nre_device.hpp"

namespace nre
{
    class FirstApp
    {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();

    private:
        NreWindow NreWindow{WIDTH, HEIGHT, "Nemesis Rendering Engine"};
        NreDevice nreDevice{NreWindow};
        NrePipeline NrePipeline{nreDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", NrePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };

}