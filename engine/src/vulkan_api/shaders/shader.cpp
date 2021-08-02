#include "vulkan_api/shaders/shader.h"

#include "platform/filesystem.h"

namespace engine
{
    Shader::Shader(const std::string &filename)
        : m_Filename(filename)
    {
        fs::path::Get(fs::path::Type::Shaders, "test");
    }

    Shader::~Shader()
    {
    }
}
