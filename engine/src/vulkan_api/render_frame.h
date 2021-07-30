#pragma once

#include "vulkan_api/device.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/fence_pool.h"
#include "vulkan_api/semaphore_pool.h"

namespace engine
{
    class RenderFrame
    {
    public:
        RenderFrame(Device &device,
                    std::unique_ptr<RenderTarget> &&render_target,
                    size_t thread_count = 1);
        ~RenderFrame();

        void UpdateRenderTarget(std::unique_ptr<RenderTarget> &&render_target)
        {
            m_SwapchainRenderTarget = std::move(render_target);
        }

        VkSemaphore RenderFrame::RequestSemaphoreWithOwnership()
        {
            return m_SemaphorePool.RequestSemaphoreWithOwnership();
        }

    private:
        Device &m_Device;
        FencePool m_FencePool;
        SemaphorePool m_SemaphorePool;
        std::unique_ptr<RenderTarget> m_SwapchainRenderTarget;
        size_t m_ThreadCount;
    };
}
