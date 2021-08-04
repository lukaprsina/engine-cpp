#pragma once

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include <filesystem>

namespace engine
{
    class Device;
    class Scene;

    class GLTFLoader
    {
    public:
        GLTFLoader(Device &device);
        ~GLTFLoader();

        std::unique_ptr<Scene> ReadSceneFromFile(const std::string &file_name, int scene_index = -1);

    private:
        Device &m_Device;
        tinygltf::Model m_Model;
        std::filesystem::path m_ModelPath;

        Scene LoadScene(int scene_index = -1);
    };
}
