#pragma once

#include "scene/script.h"

#include "common/glm.h"

namespace engine
{
    class Scene;
    class Application;
    class Window;
    class Layer;

    namespace sg
    {
        class PerspectiveCamera;

        class FreeCamera : public Script
        {
        public:
            FreeCamera(Application &application, std::string &layer_name);
            FreeCamera(FreeCamera &&other);
            FreeCamera(const FreeCamera &) = default;
            FreeCamera &operator=(const FreeCamera &) = default;
            FreeCamera &operator=(FreeCamera &&other);
            ~FreeCamera();

            void Update(float delta_time) override;
            void Resize(sg::PerspectiveCamera &camera, uint32_t width, uint32_t height);

        private:
            Application &m_App;
            std::string m_LayerName{};
            float m_SpeedMultiplier{3.0f};
            glm::vec2 m_MouseMoveDelta{};
        };
    }
}
