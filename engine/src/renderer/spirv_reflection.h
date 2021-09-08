#pragma once

ENG_DISABLE_WARNINGS()
#include <spirv_glsl.hpp>
ENG_ENABLE_WARNINGS()

#include "renderer/shader.h"

namespace engine
{
    class SPIRVReflection
    {
    public:
        bool ReflectShaderResources(VkShaderStageFlagBits stage,
                                    const std::vector<uint32_t> &spirv,
                                    std::vector<ShaderResource> &resources,
                                    const ShaderVariant &variant);

    private:
        void ParseShaderResources(const spirv_cross::Compiler &compiler,
                                  VkShaderStageFlagBits stage,
                                  std::vector<ShaderResource> &resources,
                                  const ShaderVariant &variant);

        void ParsePushConstants(const spirv_cross::Compiler &compiler,
                                VkShaderStageFlagBits stage,
                                std::vector<ShaderResource> &resources,
                                const ShaderVariant &variant);

        void ParseSpecializationConstants(const spirv_cross::Compiler &compiler,
                                          VkShaderStageFlagBits stage,
                                          std::vector<ShaderResource> &resources,
                                          const ShaderVariant &variant);
    };
}
