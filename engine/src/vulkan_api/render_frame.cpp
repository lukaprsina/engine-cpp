#include "vulkan_api/render_frame.h"

namespace engine
{
  RenderFrame::RenderFrame(Device &device,
                           std::unique_ptr<RenderTarget> &&render_target,
                           size_t thread_count)
      : m_Device(device),
        m_FencePool(device),
        m_SemaphorePool(device),
        m_SwapchainRenderTarget(std::move(render_target)),
        m_ThreadCount(thread_count)
  {
  }

  RenderFrame::~RenderFrame()
  {
  }
}
