#pragma once

namespace engine
{
    class CommandPool;
    class CommandBuffer
    {
    public:
        CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level);
        ~CommandBuffer();
        CommandBuffer(CommandBuffer &&other);

        enum class ResetMode
        {
            ResetPool,
            ResetIndividually,
            AlwaysAllocate,
        };

        enum class State
        {
            Invalid,
            Initial,
            Recording,
            Executable,
        };

        VkResult Begin(VkCommandBufferUsageFlags flags, CommandBuffer *primary_cmd_buf = nullptr);
        VkResult End();

        bool IsRecording() const { return (m_State == State::Recording); }

        VkResult Reset(ResetMode reset_mode);

        const VkCommandBuffer &GetHandle() const { return m_Handle; }

        const VkCommandBufferLevel m_Level;

    private:
        State m_State{State::Initial};
        CommandPool &m_CommandPool;
        VkCommandBuffer m_Handle{VK_NULL_HANDLE};
        uint32_t m_MaxPushConstantsSize{0};
        bool m_UpdateAfterBind{false};
    };
}
