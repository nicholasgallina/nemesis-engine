

#pragma once

#include "nre_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace nre
{

    class NreDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(NreDevice &nreDevice) : nreDevice{nreDevice} {}

            // returns a reference to itself
            // easy to chain addBinding() calls then finish withh build()
            // to get an instance of a DescriptorSetLayout

            Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<NreDescriptorSetLayout> build() const;

        private:
            NreDevice &nreDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        NreDescriptorSetLayout(
            NreDevice &nreDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~NreDescriptorSetLayout();
        NreDescriptorSetLayout(const NreDescriptorSetLayout &) = delete;
        NreDescriptorSetLayout &operator=(const NreDescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        NreDevice &nreDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class NreDescriptorWriter;
    };

    class NreDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(NreDevice &nreDevice) : nreDevice{nreDevice} {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<NreDescriptorPool> build() const;

        private:
            NreDevice &nreDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        NreDescriptorPool(
            NreDevice &nreDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~NreDescriptorPool();
        NreDescriptorPool(const NreDescriptorPool &) = delete;
        NreDescriptorPool &operator=(const NreDescriptorPool &) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void resetPool();

    private:
        NreDevice &nreDevice;
        VkDescriptorPool descriptorPool;

        friend class NreDescriptorWriter;
    };

    class NreDescriptorWriter
    {
    public:
        NreDescriptorWriter(NreDescriptorSetLayout &setLayout, NreDescriptorPool &pool);

        NreDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        NreDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        NreDescriptorSetLayout &setLayout;
        NreDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

} // namespace nre
