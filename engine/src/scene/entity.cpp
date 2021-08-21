#include "scene/entity.h"

namespace engine
{
    Entity::Entity()
        : m_Handle(entt::null), m_Scene(nullptr)
    {
    }

    Entity::Entity(entt::entity handle, Scene *scene)
        : m_Handle(handle), m_Scene(scene)
    {
    }

    Entity::~Entity()
    {
    }

    Entity::Entity(const Entity &other)
        : m_Handle(other.m_Handle),
          m_Scene(other.m_Scene)
    {
    }

    /* Entity &Entity::operator=(const Entity &other)
    {
        return *this = Entity(other);
    } */
}
