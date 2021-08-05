#pragma once

ENG_DISABLE_WARNINGS()
#include "common/glm.h"
ENG_ENABLE_WARNINGS()

namespace engine
{
    enum class LightType
    {
        Directional,
        Point,
        Spot,
        // Insert new lightype here
        Max
    };

    struct LightProperties
    {
        glm::vec3 direction{0.0f, 0.0f, -1.0f};
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity{1.0f};
        float range{0.0f};
        float inner_cone_angle{0.0f};
        float outer_cone_angle{0.0f};
    };

    class Light
    {
    public:
        Light(const std::string &name);
        ~Light();

        void SetType(const LightType &type) { m_Type = type; }
        const LightType &GetLightType() const { return m_Type; }
        void SetProperties(const LightProperties &properties) { m_Properties = properties; }
        const LightProperties &GetLightProperties() const { return m_Properties; }

    private:
        std::string m_Name{"Light"};
        LightType m_Type;
        LightProperties m_Properties;
    };
}
