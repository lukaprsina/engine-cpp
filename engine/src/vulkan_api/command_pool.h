#pragma once

#include "vulkan_api/command_buffer.h"

namespace engine
{
    class Device;
    class RenderFrame;
    class CommandPool
    {
    public:
        CommandPool(Device &device, uint32_t queue_family_index,
                    RenderFrame *render_frame = nullptr,
                    size_t thread_index = 0,
                    CommandBuffer::ResetMode reset_mode = CommandBuffer::ResetMode::ResetPool);
        ~CommandPool();

        VkResult ResetPool();
        CommandBuffer &RequestCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        Device &GetDevice() { return m_Device; }
        VkCommandPool &GetHandle() { return m_Handle; }
        size_t GetThreadIndex() const { return m_ThreadIndex; }
        CommandBuffer::ResetMode &GetResetMode() { return m_ResetMode; }

    private:
        Device &m_Device;
        VkCommandPool m_Handle{VK_NULL_HANDLE};

        RenderFrame *m_RenderFrame{};
        size_t m_ThreadIndex{0};
        std::vector<std::unique_ptr<CommandBuffer>> m_PrimaryCommandBuffers{};
        uint32_t m_ActivePrimaryCommandBufferCount{0};

        std::vector<std::unique_ptr<CommandBuffer>> m_SecondaryCommandBuffers{};
        uint32_t m_ActiveSecondaryCommandBufferCount{0};

        CommandBuffer::ResetMode m_ResetMode{CommandBuffer::ResetMode::ResetPool};
        VkResult ResetCommandBuffers();
    };
}