#pragma once

namespace engine
{
    class Device;
    class Queue;

    class QueueFamily
    {
    public:
        QueueFamily(Device &device,
                    uint32_t family_index,
                    VkQueueFamilyProperties properties,
                    VkBool32 can_present);
        ~QueueFamily();

        const std::vector<Queue> &GetQueues() const { return m_Queues; }
        uint32_t GetFamilyIndex() const { return m_FamilyIndex; }
        VkQueueFamilyProperties GetProperties() const { return m_Properties; }
        VkBool32 CanPresent() const { return m_CanPresent; }
        void SetCanPresent(VkBool32 can_present) { m_CanPresent = can_present; }

    private:
        Device &m_Device;
        std::vector<Queue> m_Queues{};
        uint32_t m_FamilyIndex{0};
        VkBool32 m_CanPresent{false};
        VkQueueFamilyProperties m_Properties{};
    };
}
