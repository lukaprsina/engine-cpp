#pragma once

#include "vulkan_api/subpasses/geometry_subpass.h"

namespace engine
{
    class ForwardSubpass : public GeometrySubpass
    {
    public:
        ForwardSubpass(RenderContext &render_context,
                       ShaderSource &&vertex_shader,
                       ShaderSource &&fragment_shader,
                       Scene &scene, Camera &camera);
        ~ForwardSubpass();

        virtual void Prepare() override;
    };
}
