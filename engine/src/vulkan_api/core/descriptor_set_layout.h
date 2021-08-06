#pragma once

namespace engine
{
    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout();
        ~DescriptorSetLayout();

        VkDescriptorSetLayout GetHandle() const { return m_Handle; }

    private:
        VkDescriptorSetLayout m_Handle{VK_NULL_HANDLE};
    };
}
