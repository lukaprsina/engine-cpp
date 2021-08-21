#include "scene/components/camera.h"

namespace engine
{
    namespace sg
    {
        Camera::Camera(const std::string &name)
            : m_Name(name)
        {
        }

        Camera::~Camera()
        {
        }
    }
}
