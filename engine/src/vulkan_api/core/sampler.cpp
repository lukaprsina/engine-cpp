#include "vulkan_api/core/sampler.h"

#include "vulkan_api/device.h"

namespace engine
{
    namespace core
    {
        Sampler::Sampler(Device &device, const VkSamplerCreateInfo &info)
            : m_Device(device)
        {
            VK_CHECK(vkCreateSampler(device.GetHandle(), &info, nullptr, &m_Handle));
        }

        Sampler::~Sampler()
        {
            if (m_Handle != VK_NULL_HANDLE)
            {
                vkDestroySampler(m_Device.GetHandle(), m_Handle, nullptr);
            }
        }

        Sampler::Sampler(Sampler &&other)
            : m_Device{other.m_Device},
              m_Handle{other.m_Handle}
        {
            other.m_Handle = VK_NULL_HANDLE;
        }
    }
}
