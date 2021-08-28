#include "scene/components/transform.h"

ENG_DISABLE_WARNINGS()
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace sg
    {
        Transform::Transform(Entity &entity)
            : m_Entity(entity)
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
            auto test = glm::translate(glm::mat4(1.0), m_Translation) *
                        glm::mat4_cast(m_Rotation) *
                        glm::scale(glm::mat4(1.0), m_Scale);

            ENG_CORE_INFO(glm::to_string(glm::translate(glm::mat4(1.0), m_Translation)));
            ENG_CORE_INFO(glm::to_string(m_Rotation));
            ENG_CORE_INFO(glm::to_string(glm::mat4_cast(m_Rotation)));
            ENG_CORE_INFO(glm::to_string(glm::scale(glm::mat4(1.0), m_Scale)));
            ENG_CORE_INFO(glm::to_string(test));

            return test;
        }

        glm::mat4 Transform::GetWorldMatrix()
        {
            UpdateWorldTransform();
            ENG_CORE_INFO(glm::to_string(m_Translation));
            ENG_CORE_INFO(glm::to_string(m_Rotation));
            ENG_CORE_INFO(glm::to_string(m_Scale));

            ENG_CORE_INFO(glm::to_string(m_WorldMatrix));
            return m_WorldMatrix;
        }

        void Transform::UpdateWorldTransform()
        {
            if (!m_UpdateWorldMatrix)
                return;

            m_WorldMatrix = GetMatrix();

            m_UpdateWorldMatrix = false;
        }
    }
}
