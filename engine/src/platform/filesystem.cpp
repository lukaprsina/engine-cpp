#include "platform/filesystem.h"

#include "platform/platform.h"

namespace engine
{
    namespace fs
    {
        namespace path
        {
            const std::unordered_map<Type, std::fs::path> relative_paths = {
                {Type::Assets, "assets"},
                {Type::Shaders, "shaders"},
                {Type::Storage, "output"},
            };

            std::fs::path Get(const Type type, const std::fs::path &file)
            {
                const std::fs::path external_storage_directoy = std::fs::path(Platform::GetExternalStorageDirectory());
                auto test = std::fs::current_path();
                switch (type)
                {
                case Type::ExternalStorage:
                    return Platform::GetExternalStorageDirectory();
                case Type::Temp:
                    return Platform::GetTempDirectory();
                default:
                    auto it = relative_paths.find(type);

                    if (it == relative_paths.end())
                        throw std::runtime_error("Path enum doesn't exist, or wasn't specified in the path map");
                    else if (it->second.empty())
                        throw std::runtime_error("Path was found, but it is empty");

                    auto path = external_storage_directoy / it->second;
                    ENG_CORE_TRACE("{}", path.c_str());

                    if (!std::fs::exists(path))
                        std::fs::create_directory(path);
                }

                return external_storage_directoy / file;
            }
        }
    }
}
