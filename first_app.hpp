#pragma once

#include "nre_window.hpp"
#include "nre_device.hpp"
#include "nre_game_object.hpp"
#include "nre_renderer.hpp"

// std
#include <memory>
#include <vector>

namespace nre
{
    class FirstApp
    {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const NreWindow &) = delete;
        FirstApp &operator=(const NreWindow &) = delete;

        void run();

    private:
        void loadGameObjects();

        NreWindow nreWindow{WIDTH, HEIGHT, "Nebula Rendering Engine"};
        NreDevice nreDevice{nreWindow};
        NreRenderer nreRenderer{nreWindow, nreDevice};
        std::vector<NreGameObject> gameObjects;
    };

}