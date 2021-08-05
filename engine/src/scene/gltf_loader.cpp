#include "scene/gltf_loader.h"

#include "vulkan_api/device.h"
#include "scene/scene.h"
#include "scene/components/light.h"
#include "platform/filesystem.h"

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace
{
    bool LoadImageData(tinygltf::Image *image, const int image_idx, std::string *err,
                       std::string *warn, int req_width, int req_height,
                       const unsigned char *bytes, int size, void *user_data)
    {
        return true;
    }
}

namespace engine
{
    std::unordered_map<std::string, bool> GLTFLoader::m_SupportedExtensions = {
        {KHR_LIGHTS_PUNCTUAL_EXTENSION, false}};

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

        gltf_loader.SetImageLoader(LoadImageData, this);

        std::filesystem::path gltf_file = fs::path::Get(fs::path::Type::Assets, file_name);

        bool importResult = gltf_loader.LoadASCIIFromFile(&m_Model, &err, &warn, gltf_file.c_str());

        if (!importResult)
        {
            ENG_CORE_ERROR("Failed to load gltf file {}.", gltf_file.generic_string());

            return nullptr;
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
        LoadLights(scene);
        return scene;
    }

    void GLTFLoader::CheckExtensions()
    {
        for (auto &used_extension : m_Model.extensionsUsed)
        {
            auto it = m_SupportedExtensions.find(used_extension);

            if (it == m_SupportedExtensions.end())
            {
                if (std::find(m_Model.extensionsRequired.begin(), m_Model.extensionsRequired.end(), used_extension) != m_Model.extensionsRequired.end())
                    throw std::runtime_error("Cannot load glTF file. Contains a required unsupported extension: " + used_extension);
                else
                    ENG_CORE_WARN("glTF file contains an unsupported extension, unexpected results may occur: {}", used_extension);
            }
            else
            {
                // Extension is supported, so enable it
                ENG_CORE_INFO("glTF file contains extension: {}", used_extension);
                it->second = true;
            }
        }
    }

    void GLTFLoader::LoadLights(Scene &scene)
    {
        std::vector<std::unique_ptr<Light>> light_components = ParseKHRLightsPunctual();
    }

    void GLTFLoader::LoadSamplers(Scene &scene)
    {
    }

    void GLTFLoader::LoadImages(Scene &scene)
    {
    }

    void GLTFLoader::LoadTextures(Scene &scene)
    {
    }

    void GLTFLoader::LoadMaterials(Scene &scene)
    {
    }

    void GLTFLoader::LoadMeshes(Scene &scene)
    {
    }

    void GLTFLoader::LoadCameras(Scene &scene)
    {
    }

    void GLTFLoader::LoadAnimations(Scene &scene)
    {
    }

    void GLTFLoader::LoadScenes(Scene &scene)
    {
    }

    std::vector<std::unique_ptr<Light>> GLTFLoader::ParseKHRLightsPunctual()
    {
        if (IsExtensionEnabled(KHR_LIGHTS_PUNCTUAL_EXTENSION))
        {
            if (m_Model.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) == m_Model.extensions.end() || !m_Model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Has("lights"))
            {
                return {};
            }
            auto &khr_lights = m_Model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Get("lights");

            std::vector<std::unique_ptr<Light>> light_components(khr_lights.ArrayLen());

            for (size_t light_index = 0; light_index < khr_lights.ArrayLen(); ++light_index)
            {
                auto &khr_light = khr_lights.Get(static_cast<int>(light_index));

                // Spec states a light has to have a type to be valid
                if (!khr_light.Has("type"))
                {
                    ENG_CORE_ERROR("KHR_lights_punctual extension: light {} doesn't have a type!", light_index);
                    throw std::runtime_error("Couldn't load glTF file, KHR_lights_punctual extension is invalid");
                }

                auto light = std::make_unique<Light>(khr_light.Get("name").Get<std::string>());

                LightType type;
                LightProperties properties;

                // Get type
                auto &gltf_light_type = khr_light.Get("type").Get<std::string>();
                if (gltf_light_type == "point")
                {
                    type = LightType::Point;
                }
                else if (gltf_light_type == "spot")
                {
                    type = LightType::Spot;
                }
                else if (gltf_light_type == "directional")
                {
                    type = LightType::Directional;
                }
                else
                {
                    ENG_CORE_ERROR("KHR_lights_punctual extension: light type '{}' is invalid", gltf_light_type);
                    throw std::runtime_error("Couldn't load glTF file, KHR_lights_punctual extension is invalid");
                }

                // Get properties
                if (khr_light.Has("color"))
                {
                    properties.color = glm::vec3(
                        static_cast<float>(khr_light.Get("color").Get(0).Get<double>()),
                        static_cast<float>(khr_light.Get("color").Get(1).Get<double>()),
                        static_cast<float>(khr_light.Get("color").Get(2).Get<double>()));
                }

                if (khr_light.Has("intensity"))
                {
                    properties.intensity = static_cast<float>(khr_light.Get("intensity").Get<double>());
                }

                if (type != LightType::Directional)
                {
                    properties.range = static_cast<float>(khr_light.Get("range").Get<double>());
                    if (type != LightType::Point)
                    {
                        if (!khr_light.Has("spot"))
                        {
                            ENG_CORE_ERROR("KHR_lights_punctual extension: spot light doesn't have a 'spot' property set", gltf_light_type);
                            throw std::runtime_error("Couldn't load glTF file, KHR_lights_punctual extension is invalid");
                        }

                        properties.inner_cone_angle = static_cast<float>(khr_light.Get("spot").Get("innerConeAngle").Get<double>());

                        if (khr_light.Get("spot").Has("outerConeAngle"))
                        {
                            properties.outer_cone_angle = static_cast<float>(khr_light.Get("spot").Get("outerConeAngle").Get<double>());
                        }
                        else
                        {
                            // Spec states default value is PI/4
                            properties.outer_cone_angle = glm::pi<float>() / 4.0f;
                        }
                    }
                }
                else if (type == LightType::Directional || type == LightType::Spot)
                {
                    // The spec states that the light will inherit the transform of the node.
                    // The light's direction is defined as the 3-vector (0.0, 0.0, -1.0) and
                    // the rotation of the node orients the light accordingly.
                    properties.direction = glm::vec3(0.0f, 0.0f, -1.0f);
                }

                light->SetType(type);
                light->SetProperties(properties);

                light_components[light_index] = std::move(light);
            }

            return light_components;
        }
        else
        {
            return {};
        }
    }

    bool GLTFLoader::IsExtensionEnabled(const std::string &requested_extension)
    {
        auto it = m_SupportedExtensions.find(requested_extension);

        if (it != m_SupportedExtensions.end())
            return it->second;
        else
            return false;
    }
}
