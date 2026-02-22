#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "nre_buffer.hpp"
#include "nre_camera.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"

#define GLM_FORCE_RADIANS // no matter the system, GLM expects radians
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <numeric>
#include <stdexcept>

namespace nre {

struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
  glm::vec4 lightPosition{-1.f};
  glm::vec4 lightColor{1.f}; // w is light intensity
};

FirstApp::FirstApp() {
  globalPool = NreDescriptorPool::Builder(nreDevice)
                   .setMaxSets(NreSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                NreSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .build();
  loadGameObjects();
}

FirstApp::~FirstApp() {}

// game loop
void FirstApp::run() {
  std::vector<std::unique_ptr<NreBuffer>> uboBuffers(
      NreSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<NreBuffer>(
        nreDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }

  auto globalSetLayout = NreDescriptorSetLayout::Builder(nreDevice)
                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_ALL_GRAPHICS)
                             .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(
      NreSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    NreDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem SimpleRenderSystem{
      nreDevice, nreRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  PointLightSystem pointLightSystem{nreDevice,
                                    nreRenderer.getSwapChainRenderPass(),
                                    globalSetLayout->getDescriptorSetLayout()};
  NreCamera camera{};
  // camera.setViewDirection(glm::vec3(0.f), glm::vec3(1.f, 0.f, 1.f));
  camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

  // doesn't have a model
  // won't be rendered
  // used to store camera's current state
  auto viewerObject = NreGameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
  KeyboardMovementController cameraController{};

  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!nreWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();
    currentTime = newTime;

    // frameTime = glm::min(frameTime, MAX_FRAME_TIME);

    cameraController.moveInPlaneXZ(nreWindow.getGLFWwindow(), frameTime,
                                   viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation,
                      viewerObject.transform.rotation);

    float aspect = nreRenderer.getAspectRatio();
    // placed inside so it's updated when the aspect ratio changes
    // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

    // beginFrame returns a null ptr if SwapChain needs to be recreated
    // beginFrame and beginSwapChainRenderPass are separate functions
    // to retain control for future extendability
    // ie: multiple renderPass(es) for reflection, shadow, post-processing
    if (auto commandBuffer = nreRenderer.beginFrame()) {
      int frameIndex = nreRenderer.getFrameIndex();
      FrameInfo frameInfo{frameIndex,
                          frameTime,
                          commandBuffer,
                          camera,
                          globalDescriptorSets[frameIndex],
                          gameObjects};

      // update
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      nreRenderer.beginSwapChainRenderPass(commandBuffer);
      SimpleRenderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);
      nreRenderer.endSwapChainRenderPass(commandBuffer);
      nreRenderer.endFrame();
    }
  }

  // CPU will block until all GPU operations have been completed
  vkDeviceWaitIdle(nreDevice.device());
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<NreModel> nreModel =
      NreModel::createModelFromFile(nreDevice, "models/flat_vase.obj");
  auto flatVase = NreGameObject::createGameObject();
  flatVase.model = nreModel;
  flatVase.transform.translation = {-.5f, .5f, 0.f};
  flatVase.transform.scale = glm::vec3(3.f);
  gameObjects.emplace(flatVase.getId(), std::move(flatVase));

  nreModel = NreModel::createModelFromFile(nreDevice, "models/smooth_vase.obj");
  auto smoothVase = NreGameObject::createGameObject();
  smoothVase.model = nreModel;
  smoothVase.transform.translation = {.5f, .5f, 0.f};
  smoothVase.transform.scale = glm::vec3(3.f);
  gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

  nreModel = NreModel::createModelFromFile(nreDevice, "models/quad.obj");
  auto floor = NreGameObject::createGameObject();
  floor.model = nreModel;
  floor.transform.translation = {0.f, .5f, 0.f};
  floor.transform.scale = glm::vec3(3.f, 1.f, 3.f);
  gameObjects.emplace(floor.getId(), std::move(floor));
};
} // namespace nre