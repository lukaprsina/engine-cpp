#pragma once

#include "renderer/shader.h"

namespace engine
{
    class ShaderSource;
    class RenderContext;
    class Subpass
    {
    public:
        Subpass(RenderContext &render_context,
                ShaderSource &&vertex_shader,
                ShaderSource &&fragment_shader);
        virtual ~Subpass();

        virtual void Prepare() = 0;

    protected:
        RenderContext &m_RenderContext;

    private:
        ShaderSource m_VertexShader;
        ShaderSource m_FragmentShader;
    };
}
