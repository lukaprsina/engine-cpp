#include "scene/components/submesh.h"

#include "scene/components/material.h"

namespace engine
{
    namespace sg
    {
        void Submesh::SetAttribute(const std::string &attribute_name, const VertexAttribute &attribute)
        {
            m_VertexAttributes[attribute_name] = attribute;
            ComputeShaderVariant();
        }

        void Submesh::SetMaterial(const Material &material)
        {
            m_Material = &material;
            ComputeShaderVariant();
        }

        bool Submesh::GetAttribute(const std::string &attribute_name, VertexAttribute &attribute) const
        {
            auto attrib_it = m_VertexAttributes.find(attribute_name);

            if (attrib_it == m_VertexAttributes.end())
                return false;

            attribute = attrib_it->second;

            return true;
        }

        void Submesh::ComputeShaderVariant()
        {
            m_ShaderVariant.Clear();

            if (m_Material != nullptr)
            {
                for (auto &texture : m_Material->m_Textures)
                {
                    std::string tex_name = texture.first;
                    std::transform(tex_name.begin(), tex_name.end(), tex_name.begin(), ::toupper);

                    m_ShaderVariant.AddDefine("HAS_" + tex_name);
                }
            }

            for (auto &attribute : m_VertexAttributes)
            {
                std::string attrib_name = attribute.first;
                std::transform(attrib_name.begin(), attrib_name.end(), attrib_name.begin(), ::toupper);
                m_ShaderVariant.AddDefine("HAS_" + attrib_name);
            }

            // ENG_CORE_TRACE("{}", m_ShaderVariant.GetPreamble());
        }
    }
}
