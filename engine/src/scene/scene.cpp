#include "scene/scene.h"

#include "scene/entity.h"
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

    void Scene::CreateRenderPipeline(Device &device)
    {
        ShaderSource vert_shader("base.vert");
        ShaderSource frag_shader("base.frag");

        auto scene_subpass = std::make_unique<ForwardSubpass>(std::move(vert_shader),
                                                              std::move(frag_shader),
                                                              *this);

        m_RenderPipeline = std::make_unique<RenderPipeline>(device);
        m_RenderPipeline->AddSubpass(std::move(scene_subpass));
    }

    void Scene::Draw(RenderContext &render_context, CommandBuffer &command_buffer, RenderTarget &render_target,
                     VkSubpassContents contents)
    {
        if (m_RenderPipeline)
            m_RenderPipeline->Draw(render_context, command_buffer,
                                   render_target);
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
