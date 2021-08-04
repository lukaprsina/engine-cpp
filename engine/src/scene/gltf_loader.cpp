#include "scene/gltf_loader.h"

#include "vulkan_api/device.h"
#include "scene/scene.h"
#include "platform/filesystem.h"

namespace engine
{
    GLTFLoader::GLTFLoader(Device &device)
        : m_Device(device)
    {
    }

    GLTFLoader::~GLTFLoader()
    {
    }

    std::unique_ptr<Scene> GLTFLoader::ReadSceneFromFile(const std::string &file_name, int scene_index)
    {
        std::string err;
        std::string warn;

        tinygltf::TinyGLTF gltf_loader;

        std::filesystem::path gltf_file = fs::path::Get(fs::path::Type::Assets, file_name);

        bool importResult = gltf_loader.LoadASCIIFromFile(&m_Model, &err, &warn, gltf_file.c_str());

        if (!importResult)
        {
            ENG_CORE_ERROR("Failed to load gltf file {}.", gltf_file.generic_string());

            // return nullptr;
        }

        if (!err.empty())
        {
            ENG_CORE_ERROR("Error loading gltf model: {}.", err.c_str());

            return nullptr;
        }

        if (!warn.empty())
        {
            ENG_CORE_WARN("{}", warn.c_str());
        }

        return std::make_unique<Scene>(LoadScene(scene_index));
    }

    Scene GLTFLoader::LoadScene(int scene_index)
    {
        auto scene = Scene();
        return scene;
    }
}
