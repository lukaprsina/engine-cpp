#pragma once

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include "scene/entity.h"

namespace engine
{
    class Device;
    class Scene;

    namespace sg
    {
        class Sampler;
        class Light;
        class Image;
        class Texture;
        class PBRMaterial;
        class Mesh;
        class Submesh;
        class Camera;
        class Transform;
    }

    namespace core
    {
        class Buffer;
    }

    class GLTFLoader
    {
    public:
        GLTFLoader(Device &device);
        ~GLTFLoader();

        std::unique_ptr<Scene> ReadSceneFromFile(const std::string &file_name, bool binary, int scene_index = -1);

    private:
        Device &m_Device;
        tinygltf::Model m_Model;
        std::filesystem::path m_ModelPath;
        static std::unordered_map<std::string, bool> m_SupportedExtensions;
        std::vector<core::Buffer> m_TransientBuffers;

        void LoadScene(int scene_index = -1);

        void CheckExtensions();
        void LoadScenes(int scene_index);
        void LoadLights();
        void LoadSamplers();
        void LoadImages();
        void LoadTextures();
        void LoadMaterials();
        void LoadMeshes();
        void LoadCameras();
        void LoadAnimations();
        void LoadNodes();

        std::unique_ptr<Scene> m_Scene;

        std::vector<std::unique_ptr<Entity>> ParseKHRLightsPunctual();
        std::unique_ptr<sg::Sampler> ParseSampler(const tinygltf::Sampler &gltf_sampler) const;
        std::unique_ptr<sg::Image> ParseImage(tinygltf::Image &gltf_image) const;
        std::unique_ptr<sg::Texture> ParseTexture(const tinygltf::Texture &gltf_texture) const;
        std::unique_ptr<sg::PBRMaterial> ParseMaterial(const tinygltf::Material &gltf_material) const;
        Entity ParseMesh(const tinygltf::Mesh &gltf_mesh) const;
        Entity ParseCamera(const tinygltf::Camera &gltf_camera) const;
        void ParseNode(tinygltf::Node &gltf_node, Entity &entity);

        bool IsExtensionEnabled(const std::string &requested_extension);
        std::unique_ptr<sg::Sampler> CreateDefaultSampler();
        std::unique_ptr<sg::PBRMaterial> CreateDefaultMaterial();
    };
}
