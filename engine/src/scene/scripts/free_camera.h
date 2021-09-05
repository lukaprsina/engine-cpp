#pragma once

#include "scene/script.h"

#include "common/glm.h"

namespace engine
{
    class Scene;

    namespace sg
    {
        class PerspectiveCamera;

        class FreeCamera : public Script
        {
        public:
            FreeCamera() = default;
            FreeCamera(Scene &scene);
            FreeCamera(FreeCamera &&other);
            FreeCamera(const FreeCamera &) = default;
            FreeCamera &operator=(const FreeCamera &) = default;
            FreeCamera &operator=(FreeCamera &&other);
            ~FreeCamera();

            void Update(float delta_time) override;
            void Resize(uint32_t width, uint32_t height) override;

        private:
            Scene &m_Scene;
            float m_SpeedMultiplier{3.0f};
            glm::vec2 m_MouseMoveDelta{};
        };
    }
}
