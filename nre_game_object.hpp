#pragma once

#include "nre_model.hpp"

// std
#include <memory>

namespace nre
{

    struct Transform2dComponent
    {
        glm::vec2 translation{}; // position offset
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2()
        {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotMatrix{{c, s}, {-s, c}};
            glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
            return rotMatrix * scaleMat;
        }
    };

    // a game object is anything with properties and methods
    class NreGameObject
    {
    public:
        // "using" keyword invokes in alias; a new name to an existing type
        using id_t = unsigned int;

        static NreGameObject createGameObject()
        {
            static id_t currentId = 0;
            return NreGameObject{currentId++};
        }

        NreGameObject(const NreGameObject &) = delete;
        NreGameObject &operator=(const NreGameObject &) = delete;
        NreGameObject(NreGameObject &&) = default;
        NreGameObject &operator=(NreGameObject &&) = default;

        const id_t getId() { return id; }

        std::shared_ptr<NreModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d{};

    private:
        NreGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
} // namespace nre