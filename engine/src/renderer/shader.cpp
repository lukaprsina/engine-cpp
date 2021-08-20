#include "renderer/shader.h"

#include "platform/filesystem.h"
#include "vulkan_api/device.h"
#include "renderer/glsl_compiler.h"
#include "renderer/spirv_reflection.h"

namespace engine
{

    inline std::vector<uint8_t> ConvertToBytes(std::vector<std::string> &lines)
    {
        std::vector<uint8_t> bytes;

        for (auto &line : lines)
        {
            line += "\n";
            std::vector<uint8_t> line_bytes(line.begin(), line.end());
            bytes.insert(bytes.end(), line_bytes.begin(), line_bytes.end());
        }

        return bytes;
    }

    inline std::vector<std::string> PrecompileShader(const std::string &source)
    {
        std::vector<std::string> final_file;

        auto lines = Split(source, '\n');

        for (auto &line : lines)
        {
            if (line.find("#include \"") == 0)
            {
                // Include paths are relative to the base shader directory
                std::string include_path = line.substr(10);
                size_t last_quote = include_path.find("\"");
                if (!include_path.empty() && last_quote != std::string::npos)
                {
                    include_path = include_path.substr(0, last_quote);
                }

                auto include_file = PrecompileShader(fs::ReadTextFile(fs::path::Get(fs::path::Type::Shaders, include_path)));
                for (auto &include_file_line : include_file)
                {
                    final_file.push_back(include_file_line);
                }
            }
            else
            {
                final_file.push_back(line);
            }
        }

        return final_file;
    }

    ShaderSource::ShaderSource(const std::string &filename)
        : m_Filename(filename)
    {
        m_FileContent = fs::ReadTextFile(fs::path::Get(fs::path::Type::Shaders, filename));

        std::hash<std::string> hasher;
        m_ID = hasher(std::string{m_FileContent.cbegin(), m_FileContent.cend()});
    }

    ShaderSource::~ShaderSource()
    {
    }

    void ShaderSource::SetFileContent(const std::string &file_content)
    {
        m_FileContent = file_content;
        std::hash<std::string> hasher;
        m_ID = hasher(std::string{m_FileContent.cbegin(), m_FileContent.cend()});
    }

    ShaderVariant::ShaderVariant(std::string &&preamble,
                                 std::vector<std::string> &&processes)
        : m_Preamble{std::move(preamble)},
          m_Processes{std::move(processes)}
    {
        UpdateId();
    }

    ShaderVariant::~ShaderVariant()
    {
        test = 65;
        ENG_ASSERT(m_ID != 4187935634071280513);
        ENG_CORE_TRACE("destroying preamble:\n{}", m_Preamble);
    }

    void ShaderVariant::AddDefine(const std::string &definition)
    {
        m_Processes.push_back("D" + definition);

        std::string tmp_def = definition;

        // The "=" needs to turn into a space
        size_t pos_equal = tmp_def.find_first_of("=");
        if (pos_equal != std::string::npos)
        {
            tmp_def[pos_equal] = ' ';
        }

        m_Preamble.append("#define " + tmp_def + "\n");

        UpdateId();
    }

    void ShaderVariant::AddDefine(const std::vector<std::string> &definitions)
    {
        for (auto &definition : definitions)
            AddDefine(definition);
    }

    void ShaderVariant::AddUndefine(const std::string &undefinition)
    {
        m_Processes.push_back("U" + undefinition);
        m_Preamble.append("#undef " + undefinition + "\n");

        UpdateId();
    }

    void ShaderVariant::AddUndefine(const std::vector<std::string> &undefinitions)
    {
        for (auto &undefinition : undefinitions)
            AddUndefine(undefinition);
    }

    void ShaderVariant::AddRuntimeArraySize(const std::string &runtime_array_name, size_t size)
    {
        if (m_RuntimeArraySizes.find(runtime_array_name) == m_RuntimeArraySizes.end())
            m_RuntimeArraySizes.insert({runtime_array_name, size});
        else
            m_RuntimeArraySizes[runtime_array_name] = size;
    }

    void ShaderVariant::Clear()
    {
        m_Preamble.clear();
        m_Processes.clear();
        m_RuntimeArraySizes.clear();
        UpdateId();
    }

    void ShaderVariant::UpdateId()
    {
        std::hash<std::string> hasher{};
        m_ID = hasher(m_Preamble);
    }

    ShaderModule::ShaderModule(Device &device,
                               VkShaderStageFlagBits stage,
                               const ShaderSource &shader_source,
                               const std::string &entry_point,
                               const ShaderVariant &shader_variant)
        : m_Device(device), m_Stage(stage),
          m_EntryPoint(entry_point)
    {
        if (entry_point.empty())
            throw VulkanException(VK_ERROR_INITIALIZATION_FAILED);

        const std::string &glsl = shader_source.GetFileContent();

        if (glsl.empty())
            throw VulkanException(VK_ERROR_INITIALIZATION_FAILED);

        auto glsl_final_source = PrecompileShader(glsl);

        GLSLCompiler glsl_compiler;

        bool is_compiled = glsl_compiler.CompileToSPIRV(stage, ConvertToBytes(glsl_final_source),
                                                        entry_point, shader_variant, m_Spirv, m_InfoLog);

        if (!is_compiled)
        {
            ENG_CORE_ERROR("Shader compilation failed for shader \"{}\"", shader_source.GetFilename());
            ENG_CORE_ERROR("{}", m_InfoLog);
            throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
        }

        SPIRVReflection spirv_reflection;

        if (!spirv_reflection.ReflectShaderResources(stage, m_Spirv,
                                                     m_Resources, shader_variant))
            throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};

        std::hash<std::string> hasher{};
        m_ID = hasher(std::string{reinterpret_cast<const char *>(m_Spirv.data()),
                                  reinterpret_cast<const char *>(m_Spirv.data() + m_Spirv.size())});
    }

    ShaderModule::~ShaderModule()
    {
    }

    ShaderModule::ShaderModule(ShaderModule &&other)
        : m_Device{other.m_Device},
          m_ID{other.m_ID},
          m_Stage{other.m_Stage},
          m_EntryPoint{other.m_EntryPoint},
          m_Spirv{other.m_Spirv},
          m_Resources{other.m_Resources},
          m_InfoLog{other.m_InfoLog}
    {
        other.m_Stage = {};
    }
}
