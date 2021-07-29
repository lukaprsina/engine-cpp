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

    private:
        Device &m_Device;
        FencePool m_FencePool;
        SemaphorePool m_SemaphorePool;
        std::unique_ptr<RenderTarget> m_SwapchainRenderTarget;
        size_t m_ThreadCount;
    };
}
