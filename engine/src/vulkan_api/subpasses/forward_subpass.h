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
        ForwardSubpass(ShaderSource &&vertex_shader,
                       ShaderSource &&fragment_shader,
                       Scene &scene);
        ~ForwardSubpass();

        virtual void Prepare(Device &device) override;
        virtual void Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer) override;
    };
}
