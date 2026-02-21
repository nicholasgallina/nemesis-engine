#pragma once

#include "nre_camera.hpp"
#include "nre_game_object.hpp"

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
        NreGameObject::Map &gameObjects;
    };
} // namespace nre