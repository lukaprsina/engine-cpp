#pragma once

#include "scene/components/camera.h"

ENG_DISABLE_WARNINGS()
#include <glm/gtc/matrix_transform.hpp>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace sg
    {
        class PerspectiveCamera : public Camera
        {
        public:
            PerspectiveCamera(const std::string &name);

            glm::mat4 GetProjection() override
            {
                // Note: Using Revsered depth-buffer for increased precision, so Znear and Zfar are flipped
                return glm::perspective(m_Fov, m_AspectRatio,
                                        m_FarPlane, m_NearPlane);
            }

            float m_AspectRatio{1.0f};
            float m_Fov{glm::radians(60.0f)};
            float m_FarPlane{100.0};
            float m_NearPlane{0.1f};
        };
    }
}
