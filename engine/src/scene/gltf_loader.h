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
    class Light;
    namespace sg
    {
        class Sampler;
        class Image;
    }

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
        static std::unordered_map<std::string, bool> m_SupportedExtensions;

        Scene LoadScene(int scene_index = -1);

        void CheckExtensions();
        void LoadLights(Scene &scene);
        void LoadSamplers(Scene &scene);
        void LoadImages(Scene &scene);
        void LoadTextures(Scene &scene);
        void LoadMaterials(Scene &scene);
        void LoadMeshes(Scene &scene);
        void LoadCameras(Scene &scene);
        void LoadAnimations(Scene &scene);
        void LoadScenes(Scene &scene);

        std::vector<std::unique_ptr<Light>> m_Lights;
        std::vector<std::unique_ptr<sg::Sampler>> m_Samplers;
        std::vector<std::unique_ptr<sg::Image>> m_Images;

        std::vector<std::unique_ptr<Light>> ParseKHRLightsPunctual();
        std::unique_ptr<sg::Sampler> ParseSampler(const tinygltf::Sampler &gltf_sampler) const;
        std::unique_ptr<sg::Image> ParseImage(tinygltf::Image &gltf_image) const;

        bool IsExtensionEnabled(const std::string &requested_extension);
    };
}
