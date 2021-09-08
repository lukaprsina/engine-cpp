#pragma once

#include "scene/scene.h"

ENG_DISABLE_WARNINGS()
#include <entt/entt.hpp>
ENG_ENABLE_WARNINGS()

namespace engine
{
    class Entity
    {
    public:
        Entity();
        Entity(entt::entity handle, Scene *scene);
        ~Entity();

        Entity(const Entity &other);
        // Entity(Entity &&other) = default;

        // Entity &operator=(const Entity &other);
        // Entity &operator=(Entity &&other) = default;

        template <typename T, typename... Args>
        T &AddComponent(Args &&...args)
        {
            ENG_ASSERT(!HasComponent<T>() && "Entity already has component!");
            T &component = m_Scene->GetRegistry().emplace<T>(m_Handle, std::forward<Args>(args)...);
            return component;
        }

        template <typename T>
        T &GetComponent()
        {
            ENG_ASSERT(HasComponent<T>() && "Entity does not have component!");
            return m_Scene->GetRegistry().get<T>(m_Handle);
        }

        template <typename T>
        bool HasComponent()
        {
            return (m_Scene->GetRegistry().all_of<T>(m_Handle));
        }

        template <typename T>
        void RemoveComponent()
        {
            ENG_ASSERT(HasComponent<T>() && "Entity does not have component!");
            m_Scene->GetRegistry().remove<T>(m_Handle);
        }

        operator bool() const { return m_Handle != entt::null; }
        operator entt::entity() const { return m_Handle; }
        operator uint32_t() const { return (uint32_t)m_Handle; }

        bool operator==(const Entity &other) const
        {
            return (m_Handle == other.m_Handle &&
                    &m_Scene == &other.m_Scene);
        }

        bool operator!=(const Entity &other) const
        {
            return !(*this == other);
        }

        entt::entity GetHandle() { return m_Handle; }
        Scene &GetScene() { return *m_Scene; }

    private:
        entt::entity m_Handle{entt::null};
        Scene *m_Scene;
    };

}