#pragma once

namespace engine
{
    class PipelineLayout
    {
    public:
        PipelineLayout();
        ~PipelineLayout();

        VkPipelineLayout GetHandle() const { return m_Handle; }

    private:
        VkPipelineLayout m_Handle{VK_NULL_HANDLE};
    };
}
