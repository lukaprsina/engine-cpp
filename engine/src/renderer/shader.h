#pragma once

namespace engine
{
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
}
