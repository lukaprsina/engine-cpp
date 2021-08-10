#pragma once

#include "entt/entt.hpp"

namespace engine
{
    class Device;

    enum class ShaderResourceMode
    {
        Static,
        Dynamic,
        UpdateAfterBind
    };

    enum class ShaderResourceType
    {
        Input,
        InputAttachment,
        Output,
        Image,
        ImageSampler,
        ImageStorage,
        Sampler,
        BufferUniform,
        BufferStorage,
        PushConstant,
        SpecializationConstant,
        All
    };

    struct ShaderResourceQualifiers
    {
        enum : uint32_t
        {
            None = 0,
            NonReadable = 1,
            NonWritable = 2,
        };
    };

    struct ShaderResource
    {
        VkShaderStageFlags stages;
        ShaderResourceType type;
        ShaderResourceMode mode;
        uint32_t set;
        uint32_t binding;
        uint32_t location;
        uint32_t input_attachment_index;
        uint32_t vec_size;
        uint32_t columns;
        uint32_t array_size;
        uint32_t offset;
        uint32_t size;
        uint32_t constant_id;
        uint32_t qualifiers;
        std::string name;
    };

    class ShaderSource
    {
    public:
        ShaderSource() = default;
        ShaderSource(const std::string &filename);
        ~ShaderSource();

        void SetFileContent(const std::string &source);

        size_t GetID() const { return m_ID; }
        const std::string &GetFilename() const { return m_Filename; }
        const std::string &GetFileContent() const { return m_FileContent; }

    private:
        size_t m_ID;
        std::string m_Filename;
        std::string m_FileContent;
    };

    class ShaderVariant
    {
    public:
        ShaderVariant() = default;
        ShaderVariant(std::string &&preamble, std::vector<std::string> &&processes);

        void AddDefine(const std::string &definition);
        void AddDefine(const std::vector<std::string> &definitions);
        void AddUndefine(const std::string &undefinition);
        void AddUndefine(const std::vector<std::string> &undefinitions);
        void AddRuntimeArraySize(const std::string &runtime_array_name, size_t size);
        void SetRuntimeArraySizes(const std::unordered_map<std::string, size_t> &sizes) { m_RuntimeArraySizes = sizes; }
        void Clear();

        size_t GetID() const { return m_ID; }
        const std::string &GetPreamble() const { return m_Preamble; }
        const std::vector<std::string> &GetProcesses() const { return m_Processes; }
        const std::unordered_map<std::string, size_t> &GetRuntimeArraySizes() const { return m_RuntimeArraySizes; }

    private:
        size_t m_ID;
        std::string m_Preamble;
        std::vector<std::string> m_Processes;
        std::unordered_map<std::string, size_t> m_RuntimeArraySizes;

        void UpdateId();
    };

    class ShaderModule
    {
    public:
        ShaderModule(Device &device,
                     VkShaderStageFlagBits stage,
                     const ShaderSource &glsl_source,
                     const std::string &entry_point,
                     const ShaderVariant &shader_variant);

        ~ShaderModule();
        ShaderModule(const ShaderModule &) = delete;
        ShaderModule(ShaderModule &&other);
        ShaderModule &operator=(const ShaderModule &) = delete;
        ShaderModule &operator=(ShaderModule &&) = delete;

        size_t GetID() const { return m_ID; }
        VkShaderStageFlagBits GetStage() const { return m_Stage; }
        const std::string &GetEntryPoint() const { return m_EntryPoint; }
        const std::vector<uint32_t> &GetBinary() const { return m_Spirv; }

    private:
        Device &m_Device;
        size_t m_ID;
        VkShaderStageFlagBits m_Stage{};
        std::string m_EntryPoint;
        std::vector<uint32_t> m_Spirv;
        std::vector<ShaderResource> m_Resources;
        std::string m_InfoLog;
    };
}
