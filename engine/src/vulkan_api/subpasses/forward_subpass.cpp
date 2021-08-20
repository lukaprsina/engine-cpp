#include "vulkan_api/subpasses/forward_subpass.h"

#include "vulkan_api/render_context.h"
#include "vulkan_api/device.h"
#include "scene/components/submesh.h"
#include "scene/components/mesh.h"
#include "scene/scene.h"
#include "renderer/shader.h"

namespace engine
{
    ForwardSubpass::ForwardSubpass(RenderContext &render_context,
                                   ShaderSource &&vertex_shader,
                                   ShaderSource &&fragment_shader,
                                   Scene &scene, Camera &camera)
        : GeometrySubpass(render_context,
                          std::move(vertex_shader),
                          std::move(fragment_shader),
                          scene, camera)
    {
    }

    ForwardSubpass::~ForwardSubpass()
    {
    }

    void ForwardSubpass::Prepare()
    {
        auto &device = m_RenderContext.GetDevice();
        auto view = m_Scene.GetRegistry().view<sg::Mesh>();

        for (auto &entity : view)
        {
            auto &mesh = view.get<sg::Mesh>(entity);

            for (auto &sub_mesh : mesh.GetSubmeshes())
            {
                auto &variant = sub_mesh->GetMutShaderVariant();

                // Same as Geometry except adds lighting definitions to sub mesh variants.
                variant.AddDefine({"MAX_LIGHT_COUNT " + std::to_string(MAX_FORWARD_LIGHT_COUNT)});

                variant.AddDefine(light_type_definitions);

                device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, m_VertexShader, variant);
                device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, m_FragmentShader, variant);
            }
        }
    }

    void ForwardSubpass::Draw(CommandBuffer &command_buffer)
    {
        AllocateLights<ForwardLights>(m_Scene.GetLights(), MAX_FORWARD_LIGHT_COUNT);
        command_buffer.BindLighting(m_LightingState, 0, 4);
        GeometrySubpass::Draw(command_buffer);
    }
}
