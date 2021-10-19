#include "vulkan_api/subpasses/forward_subpass.h"

#include "vulkan_api/render_context.h"
#include "vulkan_api/device.h"
#include "core/layer.h"
#include "scene/components/submesh.h"
#include "scene/components/mesh.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "renderer/shader.h"

namespace engine
{
    ForwardSubpass::ForwardSubpass(ShaderSource &&vertex_shader,
                                   ShaderSource &&fragment_shader,
                                   Scene &scene)
        : GeometrySubpass(std::move(vertex_shader),
                          std::move(fragment_shader),
                          scene)
    {
    }

    ForwardSubpass::~ForwardSubpass()
    {
    }

    void ForwardSubpass::Prepare(Device &device)
    {
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

    void ForwardSubpass::Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer)
    {
        AllocateLights<ForwardLights>(render_context, m_Scene, MAX_FORWARD_LIGHT_COUNT);

        command_buffer.BindLighting(m_LightingState, 0, 4);
        GeometrySubpass::Draw(render_context, layer, command_buffer);
    }
}
