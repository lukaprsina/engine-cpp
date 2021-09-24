#include "core/layer.h"

#include "window/window.h"
#include "vulkan_api/render_context.h"

namespace engine
{
    Layer::Layer(Application *application)
        : m_Application(application)
    {
    }

    RenderContext &Layer::CreateRenderContext(Device &device,
                                              std::vector<VkPresentModeKHR> &present_mode_priority,
                                              std::vector<VkSurfaceFormatKHR> &surface_format_priority)
    {
        m_RenderContext = std::make_unique<RenderContext>(device,
                                                          m_Window->GetSurface(),
                                                          present_mode_priority,
                                                          surface_format_priority,
                                                          m_Window->GetSettings().width,
                                                          m_Window->GetSettings().height);

        return *m_RenderContext;
    }

    RenderContext &Layer::GetRenderContext()
    {
        ENG_ASSERT(m_RenderContext, "Render context is not valid");
        return *m_RenderContext;
    }

    void Layer::DeleteRenderContext()
    {
        m_RenderContext.reset();
    }
}