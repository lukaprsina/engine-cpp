#include "vulkan_api/subpasses/geometry_subpass.h"

namespace engine
{
    GeometrySubpass::GeometrySubpass(RenderContext &render_context, Shader &&vertex_source, Shader &&fragment_source, Scene &scene, Camera &camera)
        : Subpass(render_context, std::move(vertex_source), std::move(fragment_source))
    {
    }

    GeometrySubpass::~GeometrySubpass()
    {
    }
}
