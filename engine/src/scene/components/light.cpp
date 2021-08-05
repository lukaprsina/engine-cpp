#include "scene/components/light.h"

namespace engine
{
    Light::Light(const std::string &name)
        : m_Name(name)
    {
    }

    Light::~Light()
    {
    }
}
