#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS // no matter the system, GLM expects radians
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <stdexcept>

namespace colors {
const glm::vec3 RED = {1.0f, 0.0f, 0.0f};
const glm::vec3 GREEN = {0.0f, 1.0f, 0.0f};
const glm::vec3 BLUE = {0.0f, 0.0f, 1.0f};
const glm::vec3 YELLOW = {1.0f, 1.0f, 1.0f};
const glm::vec3 PURPLE = {0.5f, 0.0f, 0.5f};
} // namespace colors

namespace nre {

PointLightSystem::PointLightSystem(NreDevice &device, VkRenderPass renderPass,
                                   VkDescriptorSetLayout globalSetLayout)
    : nreDevice{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
  vkDestroyPipelineLayout(nreDevice.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout) {

  // want access to push constant data in both shaders
  VkPushConstantRange pushConstantRange{};
  //   pushConstantRange.stageFlags =
  //       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  //   pushConstantRange.offset = 0;
  //   pushConstantRange.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(nreDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }
};

void PointLightSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before Pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  NrePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.attributeDescriptions.clear();
  pipelineConfig.bindingDescriptions.clear();
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  nrePipeline = std::make_unique<NrePipeline>(
      nreDevice, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv",
      pipelineConfig);
}

void PointLightSystem::render(FrameInfo &frameInfo) {
  nrePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                          &frameInfo.globalDescriptorSet, 0, nullptr);

  vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

} // namespace nre