#include "vulkan_api/render_frame.h"

#include "vulkan_api/device.h"
#include "vulkan_api/queue_family.h"

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

    void RenderFrame::Reset()
    {
        VK_CHECK(m_FencePool.Wait());

        m_FencePool.Reset();

        for (auto &command_pools_per_queue : m_CommandPools)
        {
            for (auto &command_pool : command_pools_per_queue.second)
            {
                command_pool->ResetPool();
            }
        }

        //TODO
        /* for (auto &buffer_pools_per_usage : m_BufferPools)
        {
            for (auto &buffer_pool : buffer_pools_per_usage.second)
            {
                buffer_pool.first.reset();
                buffer_pool.second = nullptr;
            }
        } */

        m_SemaphorePool.Reset();
    }

    std::vector<std::unique_ptr<CommandPool>> &RenderFrame::GetCommandPools(const QueueFamily &queue_family,
                                                                            CommandBuffer::ResetMode reset_mode)

    {
        auto command_pool_it = m_CommandPools.find(queue_family.GetFamilyIndex());

        if (command_pool_it != m_CommandPools.end())
        {
            if (command_pool_it->second.at(0)->GetResetMode() != reset_mode)
            {
                m_Device.WaitIdle();

                // Delete pools
                m_CommandPools.erase(command_pool_it);
            }
            else
            {
                return command_pool_it->second;
            }
        }

        std::vector<std::unique_ptr<CommandPool>> queue_command_pools;
        for (size_t i = 0; i < m_ThreadCount; i++)
        {
            queue_command_pools.push_back(std::make_unique<CommandPool>(m_Device, queue_family.GetFamilyIndex(),
                                                                        this, i, reset_mode));
        }

        auto res_ins_it = m_CommandPools.emplace(queue_family.GetFamilyIndex(), std::move(queue_command_pools));

        if (!res_ins_it.second)
        {
            throw std::runtime_error("Failed to insert command pool");
        }

        command_pool_it = res_ins_it.first;

        return command_pool_it->second;
    }

    CommandBuffer &RenderFrame::RequestCommandBuffer(const QueueFamily &queue_family,
                                                     CommandBuffer::ResetMode reset_mode,
                                                     VkCommandBufferLevel level, size_t thread_index)
    {
        assert(thread_index < m_ThreadCount && "Thread index is out of bounds");

        auto &command_pools = GetCommandPools(queue_family, reset_mode);

        auto command_pool_it = std::find_if(command_pools.begin(), command_pools.end(), [&thread_index](std::unique_ptr<CommandPool> &cmd_pool)
                                            { return cmd_pool->GetThreadIndex() == thread_index; });

        return (*command_pool_it)->RequestCommandBuffer(level);
    }
}
