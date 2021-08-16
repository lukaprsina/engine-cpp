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

    Scene::Scene(const Scene &)
    {
    }

    template <typename T>
    void OnComponentAdded(Entity entity, T &component)
    {
    }
}
