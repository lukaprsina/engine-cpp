#pragma once

namespace engine
{
    class Device;
    class CommandBuffer;
    class QueueFamily;

    class Queue
    {
    public:
        Queue(Device &device, QueueFamily &queue_family, uint32_t index);
        ~Queue() = default;

        VkResult Submit(const CommandBuffer &command_buffer, VkFence fence) const;
        VkResult Submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const;

        VkResult Present(const VkPresentInfoKHR &present_infos) const;

        const QueueFamily &GetQueueFamily() const { return m_QueueFamily; }
        uint32_t GetIndex() const { return m_Index; }
        VkQueue GetHandle() const { return m_Handle; }

    private:
        Device &m_Device;
        QueueFamily &m_QueueFamily;
        uint32_t m_Index{0};
        VkQueue m_Handle{VK_NULL_HANDLE};
    };
}