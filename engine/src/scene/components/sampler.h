#pragma once

#include "vulkan_api/core/sampler.h"

namespace engine
{
    namespace sg
    {
        class Sampler
        {
        public:
            Sampler(const std::string &name, core::Sampler &&vk_sampler);
            ~Sampler();

            core::Sampler m_VkSampler;

        private:
            std::string m_Name;
        };
    }
}