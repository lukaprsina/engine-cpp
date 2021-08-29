#include "scene/scene.h"

#include "scene/entity.h"
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

    sg::PerspectiveCamera &Scene::AddFreeCamera(VkExtent2D extent)
    {
        auto &camera = GetDefaultCamera();

        auto entity = CreateEntity();
        auto free_camera_script = entity.AddComponent<sg::FreeCamera>(camera, *this);
        free_camera_script.Resize(extent.width, extent.height);

        return camera;
    }

    sg::PerspectiveCamera &Scene::GetDefaultCamera()
    {
        auto view = m_Registry.view<sg::PerspectiveCamera>();

        for (auto &entity : view)
        {
            auto &entity_camera = view.get<sg::PerspectiveCamera>(entity);
            return entity_camera;
        }

        auto entity = CreateEntity();
        auto &camera = entity.AddComponent<sg::PerspectiveCamera>("main_camera");
        entity.AddComponent<sg::Transform>(entity);
        m_Cameras.emplace_back(std::make_unique<Entity>(entity));

        return camera;
    }
}
