#pragma once

#include "vulkan_api/core/buffer.h"
#include "renderer/shader.h"

namespace engine
{
    namespace sg
    {
        class Material;

        struct VertexAttribute
        {
            VkFormat format = VK_FORMAT_UNDEFINED;
            std::uint32_t stride = 0;
            std::uint32_t offset = 0;
        };

        class Submesh
        {
        public:
            Submesh();
            ~Submesh();

            void SetAttribute(const std::string &attribute_name, const VertexAttribute &attribute);
            bool GetAttribute(const std::string &attribute_name, VertexAttribute &attribute) const;

            void SetMaterial(const Material &material);
            const Material *GetMaterial() const { return m_Material; }

            const ShaderVariant &GetShaderVariant() const { return m_ShaderVariant; }
            ShaderVariant &GetMutShaderVariant() { return m_ShaderVariant; }
            void ComputeShaderVariant();

            VkIndexType m_IndexType{};
            std::uint32_t m_IndexOffset = 0;
            std::uint32_t m_VerticesCount = 0;
            std::uint32_t m_VertexIndices = 0;
            std::unordered_map<std::string, core::Buffer> m_VertexBuffers;
            std::unique_ptr<core::Buffer> m_IndexBuffer;

        private:
            std::unordered_map<std::string, VertexAttribute> m_VertexAttributes;
            const Material *m_Material{nullptr};
            ShaderVariant m_ShaderVariant;
        };
    }
}
