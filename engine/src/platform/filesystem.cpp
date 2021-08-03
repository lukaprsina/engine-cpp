#include "platform/filesystem.h"

#include "platform/platform.h"

#include <fstream>

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

            std::fs::path Get(const Type type, const std::string &filename)
            {
                std::fs::path path;

                switch (type)
                {
                case Type::ExternalStorage:
                    path = Platform::GetExternalStorageDirectory();
                    break;
                case Type::Temp:
                    path = Platform::GetTempDirectory();
                    break;
                case Type::WorkingDirectory:
                    path = std::fs::current_path();
                    break;
                default:
                    auto it = relative_paths.find(type);

                    if (it == relative_paths.end())
                        throw std::runtime_error("Path enum doesn't exist, or wasn't specified in the path map");
                    else if (it->second.empty())
                        throw std::runtime_error("Path was found, but it is empty");

                    path = Platform::GetExternalStorageDirectory() / it->second;
                }

                if (!std::fs::exists(path))
                    std::fs::create_directory(path);

                return path / filename;
            }
        }

        std::string ReadTextFile(const std::fs::path &path)
        {
            // https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html

            std::ifstream file(path.c_str(), std::ios::in);
            if (file.is_open())
            {
                std::string contents;
                file.seekg(0, std::ios::end);
                contents.reserve(file.tellg());
                file.seekg(0, std::ios::beg);
                std::copy((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>(), std::back_inserter(contents));
                file.close();
                return contents;
            }
            throw std::runtime_error("Can't open file " + path.generic_string());
        }
    }
}
