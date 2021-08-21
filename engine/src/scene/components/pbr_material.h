#pragma once

#include "scene/components/material.h"

namespace engine
{
    namespace sg
    {
        class PBRMaterial : public Material
        {
        public:
            PBRMaterial(const std::string &name);
            ~PBRMaterial();

            glm::vec4 m_BaseColorFactor{0.0f, 0.0f, 0.0f, 0.0f};
            float m_MetallicFactor{0.0f};
            float m_RoughnessFactor{0.0f};
        };
    }
}