#pragma once

#include "nre_camera.hpp"
#include "nre_device.hpp"
#include "nre_frame_info.hpp"
#include "nre_game_object.hpp"
#include "nre_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace nre {
class PointLightSystem {

public:
  PointLightSystem(NreDevice &device, VkRenderPass renderPass,
                   VkDescriptorSetLayout globalSetLayout);
  ~PointLightSystem();

  PointLightSystem(const NreWindow &) = delete;
  PointLightSystem &operator=(const NreWindow &) = delete;

  void render(FrameInfo &frameInfo);

private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  NreDevice &nreDevice;
  std::unique_ptr<NrePipeline> nrePipeline;
  VkPipelineLayout pipelineLayout;
};

} // namespace nre