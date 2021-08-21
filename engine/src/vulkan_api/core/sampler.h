#pragma once

namespace engine
{
    class Device;

    namespace core
    {
        class Sampler
        {
        public:
            Sampler(Device &device, const VkSamplerCreateInfo &info);
            ~Sampler();
            Sampler(Sampler &&other);

            VkSampler GetHandle() const { return m_Handle; }

        private:
            Device &m_Device;
            VkSampler m_Handle{VK_NULL_HANDLE};
        };
    }

}