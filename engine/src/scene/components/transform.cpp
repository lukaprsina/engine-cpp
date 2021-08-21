#include "scene/components/transform.h"

#include "scene/entity.h"

ENG_DISABLE_WARNINGS()
#include <glm/gtx/matrix_decompose.hpp>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace sg
    {
        Transform::Transform(Entity &entity)
            : m_Entity(&entity)
        {
        }

        Transform::~Transform()
        {
        }

        void Transform::SetMatrix(const glm::mat4 &matrix)
        {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(matrix, m_Scale, m_Rotation, m_Translation, skew, perspective);
            m_Rotation = glm::conjugate(m_Rotation);
            InvalidateWorldMatrix();
        }

        glm::mat4 Transform::GetMatrix() const
        {
            return glm::translate(glm::mat4(1.0), m_Translation) *
                   glm::mat4_cast(m_Rotation) *
                   glm::scale(glm::mat4(1.0), m_Scale);
        }

        glm::mat4 Transform::GetWorldMatrix()
        {
            UpdateWorldTransform();
            return m_WorldMatrix;
        }

        void Transform::UpdateWorldTransform()
        {
            if (!m_UpdateWorldMatrix)
                return;

            m_WorldMatrix = GetMatrix();

            if (m_Entity)
            {
                auto &transform = m_Entity->GetComponent<sg::Transform>();
                m_WorldMatrix = transform.GetWorldMatrix() * m_WorldMatrix;
            }

            m_UpdateWorldMatrix = false;
        }
    }
}
