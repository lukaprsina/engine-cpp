#pragma once

#include "common/vulkan_common.h"

namespace engine
{
    class Device;
    class Queue
    {
    public:
        Queue(Device &device,
              uint32_t family_index,
              VkQueueFamilyProperties properties,
              VkBool32 can_present,
              uint32_t index);
        ~Queue() = default;

        VkQueueFamilyProperties GetProperties() const { return m_Properties; }
        uint32_t GetFamilyIndex() const { return m_FamilyIndex; }

    private:
        Device &m_Device;
        VkQueue m_Handle;
        uint32_t m_FamilyIndex;
        uint32_t m_Index;
        VkBool32 m_CanPresent;
        VkQueueFamilyProperties m_Properties;
    };
}