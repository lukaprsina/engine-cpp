#pragma once

#include "scene/entity.h"

#include "common/glm.h"
ENG_DISABLE_WARNINGS()
#include <glm/gtx/quaternion.hpp>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace sg
    {
        class Transform
        {
        public:
            Transform(Entity &entity);
            ~Transform();

            void SetTranslation(const glm::vec3 &translation)
            {
                m_Translation = translation;
                InvalidateWorldMatrix();
            }

            void SetRotation(const glm::quat &rotation)
            {
                m_Rotation = rotation;
                InvalidateWorldMatrix();
            }

            void SetScale(const glm::vec3 &scale)
            {
                m_Scale = scale;
                InvalidateWorldMatrix();
            }

            const glm::vec3 &GetTranslation() const { return m_Translation; }
            const glm::quat &GetRotation() const { return m_Rotation; }
            const glm::vec3 &GetScale() const { return m_Scale; }
            void SetMatrix(const glm::mat4 &matrix);
            glm::mat4 GetMatrix() const;
            glm::mat4 GetWorldMatrix();
            void InvalidateWorldMatrix() { m_UpdateWorldMatrix = true; }

        private:
            Entity m_Entity;
            glm::vec3 m_Translation = glm::vec3(0.0, 0.0, 0.0);
            glm::quat m_Rotation = glm::quat(0.0, 1.0, 0.0, 0.0);
            glm::vec3 m_Scale = glm::vec3(1.0, 1.0, 1.0);
            glm::mat4 m_WorldMatrix = glm::mat4(1.0);
            bool m_UpdateWorldMatrix = false;
            void UpdateWorldTransform();
        };
    }
}
