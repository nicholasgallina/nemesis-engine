// preprocessor directive that tells the compiler "only include this file once per translation unit"
// if .h files get included twice (directly or  transitively) -> redefinition errors
// preprocessor optimization
#pragma once

// device required because pipeline is created on a specific GPU device
#include "nre_device.hpp"

// standard libraries for file paths and dynamic arrays
#include <string>
#include <vector>

namespace nre
{

    // PipelineConfigInfo struct: all the settings for pipeline behavior
    struct PipelineConfigInfo
    {
        // defines the viewport (which region of the framebuffer to draw) &&
        //             scissor (which region to clip)
        VkPipelineViewportStateCreateInfo viewportInfo;

        // how to assemble vertices into primitives (triangle list, strip, points)
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;

        // rasterization: polygon mode (fill/wireframe), culling (back/front/none), winding order
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;

        // anti-aliasing: how many samples per pixel
        VkPipelineMultisampleStateCreateInfo multisampleInfo;

        // how to blend new fragments with what's alr in framebuffer (transparency)
        VkPipelineColorBlendAttachmentState colorBlendAttachment;

        // overall color blending config that references the above attachment
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;

        // depth testing (closer things occlude farther things) &&
        // stencil testing (masking tricks)
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        // which states can change without recreating the pipeline
        // viewport, sicssor
        std::vector<VkDynamicState> dynamicStateEnables;

        // wrapper for Vulkan
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        // describes what resources the shaders expect
        VkPipelineLayout pipelineLayout = nullptr;

        // which render pass the pipeline is compatible with, attachments(color, depth)
        VkRenderPass renderPass = nullptr;

        // which subpass within the render pass
        uint32_t subpass = 0;
    };
    class NrePipeline
    {
        // PIPELINE:
        // device, shader file paths, config
    public:
        NrePipeline(NreDevice &device, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo);

        // cleans up Vulkan objects
        ~NrePipeline();

        // NO COPYING: Pipeline owns Vulkan resources
        // copy constructor forbidden
        NrePipeline(const NrePipeline &) = delete;
        // copy assignment operator forbidden
        NrePipeline &operator=(const NrePipeline &) = delete;

        // binds a pipeline to a command buffer, this pipeline will be used for subsequent draw calls
        void bind(VkCommandBuffer commandBuffer);

        // passing by reference avoids copying a big struct
        static void defaultPipelineConfigInfo(
            PipelineConfigInfo &configInfo);

    private:
        // reads shader bytcode from disk
        static std::vector<char> readFile(const std::string &filePath);

        // handles pipeline creation, called by constructor
        void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo);

        // wraps shader bytecode in a Vulkan shader module
        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        NreDevice &nreDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
} // namespace nre