#pragma once

#include "common/glm.h"

namespace engine
{
    namespace sg
    {
        class AABB
        {
        public:
            AABB();
            AABB(const glm::vec3 &min, const glm::vec3 &max);
            ~AABB();

            void Update(const glm::vec3 &point);
            void Update(const std::vector<glm::vec3> &vertex_data, const std::vector<uint16_t> &index_data);
            void Transform(glm::mat4 &transform);
            void Reset();

            glm::vec3 GetCenter() const { return (m_Min + m_Max) * 0.5f; }
            glm::vec3 GetScale() const { return (m_Max - m_Min); }

            glm::vec3 GetMin() const { return m_Min; }
            glm::vec3 GetMax() const { return m_Max; }

        private:
            glm::vec3 m_Min;
            glm::vec3 m_Max;
        };
    }
}