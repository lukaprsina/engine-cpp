#pragma once

ENG_DISABLE_WARNINGS()
#include <glslang/Public/ShaderLang.h>
ENG_ENABLE_WARNINGS()

#include "renderer/shader.h"

namespace engine
{
    class GLSLCompiler
    {
    private:
        static glslang::EShTargetLanguage EnvTargetLanguage;
        static glslang::EShTargetLanguageVersion EnvTargetLanguageVersion;

    public:
        static void SetTargetEnviroment(glslang::EShTargetLanguage target_language,
                                        glslang::EShTargetLanguageVersion target_language_version);
        static void ResetTargetEnviroment();
        bool CompileToSPIRV(VkShaderStageFlagBits stage,
                            const std::vector<uint8_t> &glsl_source,
                            const std::string &entry_point,
                            const ShaderVariant &shader_variant,
                            std::vector<std::uint32_t> &spirv,
                            std::string &info_log);
    };
}
