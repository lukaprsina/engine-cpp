#include "vulkan_api/subpasses/forward_subpass.h"

namespace engine
{
    ForwardSubpass::ForwardSubpass(RenderContext &render_context,
                                   Shader &&vertex_shader,
                                   Shader &&fragment_shader,
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
    }
}
