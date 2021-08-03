#pragma once

#include "renderer/shader.h"

namespace engine
{
    class Shader;
    class RenderContext;
    class Subpass
    {
    public:
        Subpass(RenderContext &render_context,
                Shader &&vertex_shader,
                Shader &&fragment_shader);
        virtual ~Subpass();

        virtual void Prepare() = 0;

    private:
        RenderContext &m_RenderContext;
        Shader m_VertexShader;
        Shader m_FragmentShader;
    };
}
