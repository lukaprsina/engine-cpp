#include "renderer/shader.h"

#include "platform/filesystem.h"

#include "shaderc/shaderc.hpp"
#include "spirv_reflect.h"

namespace engine
{
    ShaderSource::ShaderSource(const std::string &filename)
        : m_Filename(filename)
    {
        m_FileContent = fs::ReadTextFile(fs::path::Get(fs::path::Type::Shaders, filename));

        std::hash<std::string> hasher;
        m_Hash = hasher(m_FileContent);
    }

    ShaderSource::~ShaderSource()
    {
    }
}
