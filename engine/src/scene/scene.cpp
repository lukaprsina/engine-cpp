#include "scene/scene.h"

#include "scene/entity.h"
#include "scene/components/image.h"
#include "scene/components/light.h"
#include "scene/components/pbr_material.h"
#include "scene/components/sampler.h"
#include "scene/components/submesh.h"
#include "scene/components/texture.h"

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
