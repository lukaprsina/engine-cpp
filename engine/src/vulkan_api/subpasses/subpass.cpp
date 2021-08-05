#include "vulkan_api/subpasses/subpass.h"

#include "vulkan_api/render_context.h"
#include "scene/components/light.h"

namespace engine
{
    const std::vector<std::string> light_type_definitions = {
        "DIRECTIONAL_LIGHT " + std::to_string(static_cast<float>(LightType::Directional)),
        "POINT_LIGHT " + std::to_string(static_cast<float>(LightType::Point)),
        "SPOT_LIGHT " + std::to_string(static_cast<float>(LightType::Spot))};

    Subpass::Subpass(RenderContext &render_context,
                     ShaderSource &&vertex_shader,
                     ShaderSource &&fragment_shader)
        : m_RenderContext(render_context),
          m_VertexShader(std::move(vertex_shader)),
          m_FragmentShader(std::move(fragment_shader))
    {
    }

    Subpass::~Subpass()
    {
    }

    void Subpass::UpdateRenderTargetAttachments(RenderTarget &render_target)
    {
        render_target.SetInputAttachments(m_InputAttachments);
        render_target.SetOutputAttachments(m_OutputAttachments);
    }
}
