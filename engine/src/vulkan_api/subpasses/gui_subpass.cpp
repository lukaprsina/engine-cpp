#include "vulkan_api/subpasses/gui_subpass.h"

#include "core/gui.h"

namespace engine
{
    GuiSubpass::GuiSubpass(ShaderSource &&vertex_shader,
                           ShaderSource &&fragment_shader,
                           Scene &scene, Gui &gui, bool owned)
        : Subpass(std::move(vertex_shader),
                  std::move(fragment_shader)),
          m_Scene(scene), m_Gui(gui), m_Owned(owned)
    {
    }

    GuiSubpass::~GuiSubpass()
    {
    }

    void GuiSubpass::Prepare(Device &device)
    {
    }

    void GuiSubpass::Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer)
    {
        if (m_Owned)
        {
        }
        // m_Gui.DrawOwned(command_buffer, layer.GetWindow());
        else
            m_Gui.Draw(command_buffer, layer.GetWindow());
    }
}
