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

        private:
            std::string m_Name;
            core::Sampler m_Sampler;
        };
    }
}