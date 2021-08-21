#include "scene/components/material.h"

namespace engine
{
    namespace sg
    {
        Material::Material(const std::string &name)
            : m_Name(name)
        {
        }

        Material::~Material()
        {
        }
    }
}
