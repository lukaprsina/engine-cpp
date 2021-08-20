#pragma once

#include "vulkan_api/subpasses/geometry_subpass.h"

#define MAX_FORWARD_LIGHT_COUNT 8

namespace engine
{
    struct alignas(16) ForwardLights
    {
        LightInfo directional_lights[MAX_FORWARD_LIGHT_COUNT];
        LightInfo point_lights[MAX_FORWARD_LIGHT_COUNT];
        LightInfo spot_lights[MAX_FORWARD_LIGHT_COUNT];
    };

    class ForwardSubpass : public GeometrySubpass
    {
    public:
        ForwardSubpass(RenderContext &render_context,
                       ShaderSource &&vertex_shader,
                       ShaderSource &&fragment_shader,
                       Scene &scene, Camera &camera);
        ~ForwardSubpass();

        virtual void Prepare() override;
        virtual void Draw(CommandBuffer &command_buffer) override;
    };
}
