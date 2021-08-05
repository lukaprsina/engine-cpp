#pragma once

namespace engine
{
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
        ShaderSource(const std::string &filename);
        ~ShaderSource();

    private:
        std::string m_Filename;
        std::string m_FileContent;
        size_t m_Hash;
    };

    class ShaderVariant
    {
    };
    // TODO
    class ShaderModule
    {
    };
}
