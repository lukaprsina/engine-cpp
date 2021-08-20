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
            void Reset();

        private:
            glm::vec3 m_Min;
            glm::vec3 m_Max;
        };
    }
}