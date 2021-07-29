#pragma once

namespace engine
{
    class CommandBuffer
    {
    public:
        CommandBuffer();
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
    };
}
