#pragma once

#include "vulkan_api/subpasses/subpass.h"

namespace engine
{
    class Gui;

    class GuiSubpass : public Subpass
    {
    public:
        GuiSubpass(ShaderSource &&vertex_shader,
                   ShaderSource &&fragment_shader,
                   Scene &scene, Gui &gui, bool owned = false);
        ~GuiSubpass();

        virtual void Prepare(Device &device) override;
        virtual void Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer) override;

    private:
        Scene &m_Scene;
        Gui &m_Gui;
        bool m_Owned;
    };
}
