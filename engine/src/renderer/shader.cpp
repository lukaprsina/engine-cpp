#include "renderer/shader.h"

#include "platform/filesystem.h"

namespace engine
{
    Shader::Shader(const std::string &filename)
        : m_Filename(filename)
    {
        m_FileContent = fs::ReadTextFile(fs::path::Get(fs::path::Type::Shaders, filename));

        std::hash<std::string> hasher;
        m_Hash = hasher(m_FileContent);
    }

    Shader::~Shader()
    {
    }
}
