#pragma once

namespace engine
{
    namespace fs
    {
        namespace path
        {
            enum class Type
            {
                Storage,
                Shaders,
                Assets,

                ExternalStorage,
                WorkingDirectory,
                Temp
            };

            extern const std::unordered_map<Type, std::fs::path> relative_paths;

            std::fs::path Get(const Type type, const std::string &filename = "");
        }

        std::string ReadTextFile(const std::fs::path &path);
    }
}