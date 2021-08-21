#pragma once

#include "common/glm.h"

namespace engine
{
    namespace sg
    {
        class Texture;

        enum class AlphaMode
        {
            Opaque,
            Mask,
            Blend
        };
        class Material
        {
        public:
            Material(const std::string &name);
            virtual ~Material();
            Material(Material &&other) = default;

            std::unordered_map<std::string, Texture *> m_Textures;
            glm::vec3 m_Emissive{0.0f, 0.0f, 0.0f};
            bool m_DoubleSided{false};
            float m_AlphaCutoff{0.5f};
            AlphaMode m_AlphaMode{AlphaMode::Opaque};

        private:
            std::string m_Name;
        };
    }
}
