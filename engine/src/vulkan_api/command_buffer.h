#pragma once

namespace engine
{
    class CommandPool;
    class CommandBuffer
    {
    public:
        CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level);
        ~CommandBuffer();

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

        VkResult Reset(ResetMode reset_mode);

        const VkCommandBufferLevel m_Level;

    private:
        State m_State{State::Initial};
        CommandPool &m_CommandPool;
        VkCommandBuffer m_Handle{VK_NULL_HANDLE};
        uint32_t m_MaxPushConstantsSize;
    };
}
