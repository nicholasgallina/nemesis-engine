#pragma once

#include "nre_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace nre {

struct TransformComponent {
  glm::vec3 translation{}; // position offset
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();
  glm::mat3 normalMatrix();
};

// a game object having this component will be the method to signal whether an
// object is compatible with the point light system

// size and position will be used from the transform component instead of
// creating duplicate fields
struct PointLightComponent {
  float lightIntensity = 1.0f;
};

// a game object is anything with properties and methods
class NreGameObject {
public:
  // "using" keyword invokes in alias; a new name to an existing type
  using id_t = unsigned int;

  // now, game objects can be looked up in constant time with id
  // not a great solution for ECS
  using Map = std::unordered_map<id_t, NreGameObject>;

  static NreGameObject createGameObject() {
    static id_t currentId = 0;
    return NreGameObject{currentId++};
  }

  static NreGameObject makePointLight(float intensity = 10.f,
                                      float radius = 0.1f,
                                      glm::vec3 color = glm::vec3(1.f));

  NreGameObject(const NreGameObject &) = delete;
  NreGameObject &operator=(const NreGameObject &) = delete;
  NreGameObject(NreGameObject &&) = default;
  NreGameObject &operator=(NreGameObject &&) = default;

  const id_t getId() { return id; }

  glm::vec3 color{};
  TransformComponent transform{};

  // optional pointer components
  std::shared_ptr<NreModel> model{};
  // use a pointer so possibility of having null value
  std::unique_ptr<PointLightComponent> pointLight = nullptr;

private:
  NreGameObject(id_t objId) : id{objId} {}

  id_t id;
};
}; // namespace nre