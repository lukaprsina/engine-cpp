#include "scene/components/perspective_camera.h"

namespace engine
{
    namespace sg
    {
        PerspectiveCamera::PerspectiveCamera(const std::string &name)
            : Camera(name)
        {
        }

        PerspectiveCamera::PerspectiveCamera(PerspectiveCamera &&other)
            : Camera(other.m_Name), m_AspectRatio(other.m_AspectRatio),
              m_Fov(other.m_Fov), m_FarPlane(other.m_FarPlane),
              m_NearPlane(other.m_NearPlane)
        {
        }

        PerspectiveCamera &PerspectiveCamera::operator=(PerspectiveCamera &&other)
        {
            return *this;
        }
    }
}
