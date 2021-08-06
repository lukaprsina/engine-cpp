#include "renderer/glsl_compiler.h"

ENG_DISABLE_WARNINGS()
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace
    {
        inline EShLanguage FindShaderLanguage(VkShaderStageFlagBits stage)
        {
            switch (stage)
            {
            case VK_SHADER_STAGE_VERTEX_BIT:
                return EShLangVertex;

            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                return EShLangTessControl;

            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                return EShLangTessEvaluation;

            case VK_SHADER_STAGE_GEOMETRY_BIT:
                return EShLangGeometry;

            case VK_SHADER_STAGE_FRAGMENT_BIT:
                return EShLangFragment;

            case VK_SHADER_STAGE_COMPUTE_BIT:
                return EShLangCompute;

            case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
                return EShLangRayGen;

            case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
                return EShLangAnyHit;

            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                return EShLangClosestHit;

            case VK_SHADER_STAGE_MISS_BIT_KHR:
                return EShLangMiss;

            case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
                return EShLangIntersect;

            case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
                return EShLangCallable;

            default:
                return EShLangVertex;
            }
        }
    }

    glslang::EShTargetLanguage GLSLCompiler::EnvTargetLanguage = glslang::EShTargetLanguage::EShTargetNone;
    glslang::EShTargetLanguageVersion GLSLCompiler::EnvTargetLanguageVersion = (glslang::EShTargetLanguageVersion)0;

    void GLSLCompiler::SetTargetEnviroment(glslang::EShTargetLanguage target_language, glslang::EShTargetLanguageVersion target_language_version)
    {
        GLSLCompiler::EnvTargetLanguage = target_language;
        GLSLCompiler::EnvTargetLanguageVersion = target_language_version;
    }

    void GLSLCompiler::ResetTargetEnviroment()
    {
        GLSLCompiler::EnvTargetLanguage = glslang::EShTargetLanguage::EShTargetNone;
        GLSLCompiler::EnvTargetLanguageVersion = (glslang::EShTargetLanguageVersion)0;
    }

    bool GLSLCompiler::CompileToSPIRV(VkShaderStageFlagBits stage,
                                      const std::vector<uint8_t> &glsl_source,
                                      const std::string &entry_point,
                                      const ShaderVariant &shader_variant,
                                      std::vector<std::uint32_t> &spirv,
                                      std::string &info_log)
    {
        glslang::InitializeProcess();

        EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

        EShLanguage language = FindShaderLanguage(stage);
        std::string source = std::string(glsl_source.begin(), glsl_source.end());

        const char *file_name_list[1] = {""};
        const char *shader_source = reinterpret_cast<const char *>(source.data());

        glslang::TShader shader(language);
        shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list, 1);
        shader.setEntryPoint(entry_point.c_str());
        shader.setSourceEntryPoint(entry_point.c_str());
        shader.setPreamble(shader_variant.GetPreamble().c_str());
        shader.addProcesses(shader_variant.GetProcesses());
        if (GLSLCompiler::EnvTargetLanguage != glslang::EShTargetLanguage::EShTargetNone)
        {
            shader.setEnvTarget(GLSLCompiler::EnvTargetLanguage, GLSLCompiler::EnvTargetLanguageVersion);
        }

        if (!shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages))
        {
            info_log = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
            return false;
        }

        // Add shader to new program object.
        glslang::TProgram program;
        program.addShader(&shader);

        // Link program.
        if (!program.link(messages))
        {
            info_log = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
            return false;
        }

        // Save any info log that was generated.
        if (shader.getInfoLog())
        {
            info_log += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
        }

        if (program.getInfoLog())
        {
            info_log += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        }

        glslang::TIntermediate *intermediate = program.getIntermediate(language);

        // Translate to SPIRV.
        if (!intermediate)
        {
            info_log += "Failed to get shared intermediate code.\n";
            return false;
        }

        spv::SpvBuildLogger logger;

        glslang::GlslangToSpv(*intermediate, spirv, &logger);

        info_log += logger.getAllMessages() + "\n";

        // Shutdown glslang library.
        glslang::FinalizeProcess();

        return true;
    }
}
