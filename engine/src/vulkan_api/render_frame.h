#pragma once

#include "vulkan_api/command_pool.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/fence_pool.h"
#include "vulkan_api/semaphore_pool.h"

namespace engine
{
    class QueueFamily;

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

        RenderTarget &GetRenderTarget() { return *m_SwapchainRenderTarget; }

        void Reset();

        VkFence RequestFence()
        {
            return m_FencePool.RequestFence();
        }

        VkSemaphore RequestSemaphore()
        {
            return m_SemaphorePool.RequestSemaphore();
        }

        VkSemaphore RequestSemaphoreWithOwnership()
        {
            return m_SemaphorePool.RequestSemaphoreWithOwnership();
        }

        void ReleaseOwnedSemaphore(VkSemaphore semaphore)
        {
            m_SemaphorePool.ReleaseOwnedSemaphore(semaphore);
        }

        CommandBuffer &RequestCommandBuffer(const QueueFamily &queue_family,
                                            CommandBuffer::ResetMode reset_mode = CommandBuffer::ResetMode::ResetPool,
                                            VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                            size_t thread_index = 0);

        SemaphorePool GetSemaphorePool() { return m_SemaphorePool; }

    private:
        Device &m_Device;

        std::vector<std::unique_ptr<CommandPool>> &GetCommandPools(const QueueFamily &queue_family, CommandBuffer::ResetMode reset_mode);

        std::map<uint32_t, std::vector<std::unique_ptr<CommandPool>>> m_CommandPools{};
        FencePool m_FencePool;
        SemaphorePool m_SemaphorePool;
        std::unique_ptr<RenderTarget> m_SwapchainRenderTarget;
        size_t m_ThreadCount{1};
        // std::map<VkBufferUsageFlags, std::vector<std::pair<BufferPool, BufferBlock *>>> buffer_pools;
    };
}
