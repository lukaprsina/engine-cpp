#pragma once

#include "scene/script.h"

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
            FreeCamera(PerspectiveCamera &perspective_camera, Scene &scene);
            FreeCamera(FreeCamera &&other);
            FreeCamera(const FreeCamera &) = default;
            FreeCamera &operator=(const FreeCamera &) = default;
            FreeCamera &operator=(FreeCamera &&other);
            ~FreeCamera();

            void Update(float delta_time) override;

        private:
            PerspectiveCamera &m_PerspectiveCamera;
            Scene &m_Scene;
            float m_SpeedMultiplier{7.0f};
        };
    }
}
