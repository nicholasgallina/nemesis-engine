#pragma once

#include "nre_device.hpp"
#include "nre_buffer.hpp"

#define GLM_FORCE_RADIANS // no matter the system, GLM expects radians
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
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
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal{};

            // shorthand for two-dimensional texture coordinates
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const
            {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string &filepath);
        };

        NreModel(NreDevice &device, const NreModel::Builder &builder);
        ~NreModel();

        NreModel(const NreModel &) = delete;
        NreModel &operator=(const NreModel &) = delete;

        static std::unique_ptr<NreModel> createModelFromFile(NreDevice &device, const std::string &filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        NreDevice &nreDevice;

        std::unique_ptr<NreBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<NreBuffer> indexBuffer;
        uint32_t indexCount;
    };
} // namespace nre