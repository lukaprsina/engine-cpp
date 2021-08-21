#include "scene/components/pbr_material.h"

namespace engine
{
    namespace sg
    {
        PBRMaterial::PBRMaterial(const std::string &name)
            : Material(name)
        {
        }

        PBRMaterial::~PBRMaterial()
        {
        }
    }
}