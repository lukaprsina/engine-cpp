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

                SourceDirectory,
                ExternalStorage,
                WorkingDirectory,
                Temp
            };

            extern const std::unordered_map<Type, std::filesystem::path> relative_paths;

            std::filesystem::path Get(const Type type, const std::string &filename = "");
        }

        std::string ReadTextFile(const std::filesystem::path &path);
    }
}