#include "scene/scene.h"

#include "scene/entity.h"
#include "core/layer.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/subpasses/forward_subpass.h"
#include "vulkan_api/device.h"
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

    void Scene::Update(float delta_time)
    {
        auto view = m_Registry.view<sg::FreeCamera>();

        for (auto &entity : view)
        {
            auto &camera = view.get<sg::FreeCamera>(entity);
            camera.Update(delta_time);
        }
    }

    Entity Scene::CreateEntity()
    {
        Entity entity{m_Registry.create(), this};
        return entity;
    }
}
