#include "scene/scene.h"

#include "scene/entity.h"

namespace engine
{
    Scene::Scene()
    {
    }

    Scene::Scene(const std::string &name)
        : m_Name(name)
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::CreateEntity()
    {
        Entity entity{m_Registry.create(), this};
        return entity;
    }

}
