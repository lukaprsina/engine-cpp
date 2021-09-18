#include "scene/scene.h"

#include "scene/entity.h"
#include "window/window.h"
#include "scene/components/image.h"
#include "scene/scripts/free_camera.h"
#include "scene/components/perspective_camera.h"
#include "scene/components/transform.h"
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

    void Scene::AddFreeCamera(VkExtent2D extent, Window *window)
    {
        m_DefaultCamera = m_Cameras[0].get();
        auto free_camera_script = m_DefaultCamera->AddComponent<sg::FreeCamera>(*this, window);
        free_camera_script.Resize(extent.width, extent.height);
        ENG_ASSERT(m_DefaultCamera->HasComponent<sg::Transform>());
    }
}
