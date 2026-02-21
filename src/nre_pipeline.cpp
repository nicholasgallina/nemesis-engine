#include "nre_pipeline.hpp"
#include "nre_device.hpp"
#include "nre_model.hpp"

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace nre {

// stores reference to logical device wrapper and immediately creates pipeline
// device reference is needed because all Vulkan object creation/destruction
// requires VkDevice
NrePipeline::NrePipeline(NreDevice &device, const std::string &vertFilepath,
                         const std::string &fragFilepath,
                         const PipelineConfigInfo &configInfo)
    : nreDevice(device) {
  createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
}

NrePipeline::~NrePipeline() {
  // In Vulkan, anything created must be manually destroyed
  // technically, shader modules can be destroyed right after pipeline creation
  // (pipeline has its own copy of compiled code), but it doesn't matter
  // bc pipeline doesn't hold references to modules after creation
  vkDestroyShaderModule(nreDevice.device(), vertShaderModule, nullptr);
  vkDestroyShaderModule(nreDevice.device(), fragShaderModule, nullptr);
  vkDestroyPipeline(nreDevice.device(), graphicsPipeline, nullptr);
}

// readFile() -> raw SPIR-V bytecode of a compiled shader
// usage: compiled shader programs ready to be loaded into GPU
std::vector<char> NrePipeline::readFile(const std::string &filePath) {
  std::string enginePath = ENGINE_DIR + filePath;
  std::ifstream file(enginePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error(">> file not opened: " + enginePath);
  }

  // tellg() returns std::streampos
  // logic: this cursor position represents a size, so it can be converted to
  // size_t to be used with std::vector
  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);

  // returns raw char pointer to vector's array
  // needed bc file.read() takes a raw pointer, not a vector
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

void NrePipeline::createGraphicsPipeline(const std::string &vertFilepath,
                                         const std::string &fragFilepath,
                                         const PipelineConfigInfo &configInfo) {
  // argument validation
  assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
         ">> pipeline not created - pipelineLayout not provided in configInfo");
  assert(configInfo.renderPass != VK_NULL_HANDLE &&
         ">> pipeline not created - renderPass not provided in configInfo");

  // load shader bytecode
  auto vertCode = readFile(vertFilepath);
  auto fragCode = readFile(fragFilepath);

  createShaderModule(vertCode, &vertShaderModule);
  createShaderModule(fragCode, &fragShaderModule);

  // array of two stages- vertex and fragment, a minimal graphics pipeline :)
  VkPipelineShaderStageCreateInfo shaderStages[2];
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  // which pipeline stage
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

  // compiled shader module handle
  shaderStages[0].module = vertShaderModule;

  // entry point func name in shader
  shaderStages[0].pName = "main";

  shaderStages[0].flags = 0;
  shaderStages[0].pNext = nullptr;

  // compile-time constants injectable into shaders, null = none
  shaderStages[0].pSpecializationInfo = nullptr;
  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = fragShaderModule;
  shaderStages[1].pName = "main";
  shaderStages[1].flags = 0;
  shaderStages[1].pNext = nullptr;
  shaderStages[1].pSpecializationInfo = nullptr;

  // here's a buffer, here's how to step through it
  auto bindingDescriptions = NreModel::Vertex::getBindingDescriptions();

  // here's where each field lives within a vertex
  auto attributeDescriptions = NreModel::Vertex::getAttributeDescriptions();
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.vertexBindingDescriptionCount =
      static_cast<uint32_t>(bindingDescriptions.size());
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

  // sets up viewport and scissor configuration
  VkPipelineViewportStateCreateInfo viewportInfo{};
  viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportInfo.viewportCount = 1;
  viewportInfo.pViewports = nullptr;
  viewportInfo.scissorCount = 1;
  viewportInfo.pScissors = nullptr;

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;

  // which shaders run (imm = immutable after creation)
  pipelineInfo.pStages = shaderStages;

  // vertex format imm
  pipelineInfo.pVertexInputState = &vertexInputInfo;

  // topology imm
  pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
  pipelineInfo.pViewportState = &viewportInfo;

  // culling, winding order, polygon mode imm
  pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;

  // MSAA imm
  pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;

  // alpha blending imm
  pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
  pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
  pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

  // descriptor and push constant layout imm
  pipelineInfo.layout = configInfo.pipelineLayout;
  pipelineInfo.renderPass = configInfo.renderPass;
  pipelineInfo.subpass = configInfo.subpass;

  pipelineInfo.basePipelineIndex = -1;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(nreDevice.device(), VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error(">> pipeline not created");
  }
}

// wraps shader bytecode into a Vulkan shader module
void NrePipeline::createShaderModule(const std::vector<char> &code,
                                     VkShaderModule *shaderModule) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  if (vkCreateShaderModule(nreDevice.device(), &createInfo, nullptr,
                           shaderModule) != VK_SUCCESS) {
    throw std::runtime_error(">> failed to create shader module");
  }
}

// binds the pipeline to a command buffer so subsequent draw calls use it
void NrePipeline::bind(VkCommandBuffer commandBuffer) {
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    graphicsPipeline);
}

void NrePipeline::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo) {
  // draw triangles, no primitve restart
  configInfo.inputAssemblyInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  // fill polygons, no culling, no depth bias
  configInfo.rasterizationInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
  configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  configInfo.rasterizationInfo.lineWidth = 1.0f;
  configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
  configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // optional
  configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // optional
  configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // optional

  // no MSAA (single sample)
  configInfo.multisampleInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
  configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  configInfo.multisampleInfo.minSampleShading = 1.0f;          // optional
  configInfo.multisampleInfo.pSampleMask = nullptr;            // optional
  configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // optional
  configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // optional

  // no blending, write to RGBA directly
  configInfo.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor =
      VK_BLEND_FACTOR_ONE; // optional
  configInfo.colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // optional
  configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // optional
  configInfo.colorBlendAttachment.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE; // optional
  configInfo.colorBlendAttachment.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ZERO;                                       // optional
  configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // optional

  // no logic ops uses the attachment above
  configInfo.colorBlendInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // optional
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // optional
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // optional
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // optional
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // optional

  // depth testing on, closer objects win, no stencil
  configInfo.depthStencilInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
  configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.minDepthBounds = 0.0f; // optional
  configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // optional
  configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
  configInfo.depthStencilInfo.front = {}; // optional
  configInfo.depthStencilInfo.back = {};  // optional

  // viewport and scissor set at draw time
  configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR};
  configInfo.dynamicStateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  configInfo.dynamicStateInfo.pDynamicStates =
      configInfo.dynamicStateEnables.data();
  configInfo.dynamicStateInfo.dynamicStateCount =
      static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
  configInfo.dynamicStateInfo.flags = 0;
}

} // namespace nre