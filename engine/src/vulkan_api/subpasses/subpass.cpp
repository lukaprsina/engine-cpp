#include "vulkan_api/subpasses/subpass.h"

#include "vulkan_api/render_context.h"

namespace engine
{
    Subpass::Subpass(RenderContext &render_context,
                     Shader &&vertex_shader,
                     Shader &&fragment_shader)
        : m_RenderContext(render_context),
          m_VertexShader(std::move(vertex_shader)),
          m_FragmentShader(std::move(fragment_shader))
    {
    }

    Subpass::~Subpass()
    {
    }
}
