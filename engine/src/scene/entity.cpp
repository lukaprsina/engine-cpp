#include "scene/entity.h"

namespace engine
{
    Entity::Entity(entt::entity handle, Scene &scene)
        : m_Handle(handle), m_Scene(scene)
    {
    }

    Entity::~Entity()
    {
    }
}
