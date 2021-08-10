#include "vulkan_api/subpasses/forward_subpass.h"

#include "vulkan_api/render_context.h"

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
        // auto &device = m_RenderContext.GetDevice();
    }

    void ForwardSubpass::Draw(CommandBuffer &command_buffer)
    {
    }
}
