#pragma once

#include "nre_device.hpp"

#define GLM_FORCE_RADIANS // no matter the system, GLM expects radians
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace nre
{
    // take vertex data created by or read in a file on CPU
    // allocate memory and copy data to device GPU for efficient rendering
    class NreModel
    {
    public:
        struct Vertex
        {
            glm::vec2 positon;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };
        NreModel(NreDevice &device, const std::vector<Vertex> &vertices);
        ~NreModel();

        NreModel(const NreModel &) = delete;
        NreModel &operator=(const NreModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        NreDevice &nreDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
} // namespace nre