#include "scene/components/sampler.h"

namespace engine
{
    namespace sg
    {
        Sampler::Sampler(const std::string &name, core::Sampler &&vk_sampler)
            : m_VkSampler(std::move(vk_sampler)), m_Name(name)
        {
        }

        Sampler::~Sampler()
        {
        }
    }
}