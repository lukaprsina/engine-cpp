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
                WorkingDirectory = ExternalStorage,
                Temp
            };

            extern const std::unordered_map<Type, std::fs::path> relative_paths;

            std::fs::path Get(const Type type, const std::fs::path &file);
        }
    }
}