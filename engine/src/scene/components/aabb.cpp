#include "scene/components/aabb.h"

namespace engine
{
    namespace sg
    {
        AABB::AABB()
        {
            Reset();
        }

        AABB::AABB(const glm::vec3 &min, const glm::vec3 &max)
            : m_Min(min), m_Max(max)
        {
        }

        AABB::~AABB()
        {
        }

        void AABB::Update(const glm::vec3 &point)
        {
            m_Min = glm::min(m_Min, point);
            m_Max = glm::max(m_Max, point);
        }

        void AABB::Update(const std::vector<glm::vec3> &vertex_data, const std::vector<uint16_t> &index_data)
        {
            if (index_data.size() > 0)
            {
                for (size_t index_id = 0; index_id < index_data.size(); index_id++)
                {
                    Update(vertex_data[index_data[index_id]]);
                }
            }
            else
            {
                for (size_t vertex_id = 0; vertex_id < vertex_data.size(); vertex_id++)
                {
                    Update(vertex_data[vertex_id]);
                }
            }
        }

        void AABB::Reset()
        {
            m_Min = std::numeric_limits<glm::vec3>::max();
            m_Max = std::numeric_limits<glm::vec3>::min();
        }
    }
}