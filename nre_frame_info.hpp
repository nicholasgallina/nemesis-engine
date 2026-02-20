#pragma once

#include "nre_camera.hpp"

// lib
#include <vulkan/vulkan.h>

namespace nre
{
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        NreCamera &camera;
        VkDescriptorSet globalDescriptorSet;
    };
} // namespace nre