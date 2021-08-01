#pragma once

#include "common/vulkan.h"

namespace engine
{
    class Device;
    class Queue
    {
    public:
        Queue(Device &device, uint32_t family_index, uint32_t index);
        ~Queue() = default;

        uint32_t GetFamilyIndex() const { return m_FamilyIndex; }
        uint32_t GetIndex() const { return m_Index; }

    private:
        Device &m_Device;
        uint32_t m_FamilyIndex{0};
        uint32_t m_Index{0};
        VkQueue m_Handle{VK_NULL_HANDLE};
    };

    class QueueFamily
    {
    public:
        QueueFamily(Device &device,
                    uint32_t family_index,
                    VkQueueFamilyProperties properties,
                    VkBool32 can_present);
        ~QueueFamily() = default;

        uint32_t GetFamilyIndex() const { return m_FamilyIndex; }
        VkQueueFamilyProperties GetProperties() const { return m_Properties; }
        uint32_t CanPresent() const { return m_CanPresent; }

    private:
        Device &m_Device;
        std::vector<Queue> m_Queues{};
        uint32_t m_FamilyIndex{0};
        VkBool32 m_CanPresent{false};
        VkQueueFamilyProperties m_Properties{};
    };
}