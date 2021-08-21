#pragma once

ENG_DISABLE_WARNINGS()
#include "common/glm.h"
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace sg
    {
        class Camera
        {
        public:
            Camera();
            virtual ~Camera();

            virtual glm::mat4 GetProjection() = 0;
            glm::mat4 m_PreRotation{1.0f};
        };
    }
}
