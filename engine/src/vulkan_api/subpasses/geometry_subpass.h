#pragma once

#include "vulkan_api/subpasses/subpass.h"

namespace engine
{
    class RenderContext;
    class ShaderSource;
    class Scene;
    class Camera;
    class Scene;

    class GeometrySubpass : public Subpass
    {
    public:
        GeometrySubpass(RenderContext &render_context,
                        ShaderSource &&vertex_shader,
                        ShaderSource &&fragment_shader,
                        Scene &scene, Camera &camera);
        virtual ~GeometrySubpass();

        virtual void Prepare() override;
        virtual void Draw(CommandBuffer &command_buffer) override;

    protected:
        Scene &m_Scene;
        uint32_t m_ThreadIndex{0};
        RasterizationState m_BaseRasterizationState{};
    };
}
