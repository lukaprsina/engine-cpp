#pragma once

namespace engine
{
    class Shader
    {
    public:
        Shader(const std::string &filename);
        ~Shader();

    private:
        std::string m_Filename;
    };
}
