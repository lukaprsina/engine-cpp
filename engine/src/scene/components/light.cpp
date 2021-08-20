#include "scene/components/light.h"

#include "scene/entity.h"

namespace engine
{
    namespace sg
    {
        Light::Light(const std::string &name, Entity entity)
            : m_Name(name), m_Entity(&entity)
        {
        }

        Light::~Light()
        {
        }
    }
}
