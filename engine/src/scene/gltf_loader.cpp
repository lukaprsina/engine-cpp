#define TINYGLTF_IMPLEMENTATION
#include "scene/gltf_loader.h"

#include "core/timer.h"
#include "platform/filesystem.h"
#include "scene/components/image.h"
#include "scene/components/image/astc.h"
#include "scene/components/light.h"
#include "scene/components/mesh.h"
#include "scene/components/pbr_material.h"
#include "scene/components/sampler.h"
#include "scene/components/submesh.h"
#include "scene/components/texture.h"
#include "scene/components/transform.h"
#include "scene/components/perspective_camera.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "vulkan_api/command_pool.h"
#include "vulkan_api/core/buffer.h"
#include "vulkan_api/device.h"
#include "vulkan_api/fence_pool.h"

#include <ThreadPool.h>
#include "common/glm.h"
ENG_DISABLE_WARNINGS()
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
ENG_ENABLE_WARNINGS()

#define KHR_LIGHTS_PUNCTUAL_EXTENSION "KHR_lights_punctual"

namespace engine
{
    namespace
    {
        bool LoadImageData(tinygltf::Image *image, const int image_idx, std::string *err,
                           std::string *warn, int req_width, int req_height,
                           const unsigned char *bytes, int size, void *user_data)
        {
            return true;
        }

        inline VkFilter FindMinFilter(int min_filter)
        {
            switch (min_filter)
            {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
                return VK_FILTER_NEAREST;
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
                return VK_FILTER_LINEAR;
            default:
                return VK_FILTER_LINEAR;
            }
        };

        inline VkSamplerMipmapMode FindMipmapMode(int min_filter)
        {
            switch (min_filter)
            {
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            }
        };

        inline VkFilter FindMagFilter(int mag_filter)
        {
            switch (mag_filter)
            {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
                return VK_FILTER_NEAREST;
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
                return VK_FILTER_LINEAR;
            default:
                return VK_FILTER_LINEAR;
            }
        };

        inline VkSamplerAddressMode FindWrapMode(int wrap)
        {
            switch (wrap)
            {
            case TINYGLTF_TEXTURE_WRAP_REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            default:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
        };

        inline void UploadImageToGpu(CommandBuffer &command_buffer, core::Buffer &staging_buffer, sg::Image &image)
        {
            // Clean up the image data, as they are copied in the staging buffer
            image.ClearData();

            {
                ImageMemoryBarrier memory_barrier{};
                memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                memory_barrier.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                memory_barrier.src_access_mask = 0;
                memory_barrier.dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_HOST_BIT;
                memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

                command_buffer.CreateImageMemoryBarrier(image.GetVkImageView(), memory_barrier);
            }

            // Create a buffer image copy for every mip level
            auto &mipmaps = image.GetMipmaps();

            std::vector<VkBufferImageCopy> buffer_copy_regions(mipmaps.size());

            for (size_t i = 0; i < mipmaps.size(); ++i)
            {
                auto &mipmap = mipmaps[i];
                auto &copy_region = buffer_copy_regions[i];

                copy_region.bufferOffset = mipmap.offset;
                copy_region.imageSubresource = image.GetVkImageView().GetSubresourceLayers();
                // Update miplevel
                copy_region.imageSubresource.mipLevel = mipmap.level;
                copy_region.imageExtent = mipmap.extent;
            }

            command_buffer.CopyBufferToImage(staging_buffer, image.GetVkImage(), buffer_copy_regions);

            {
                ImageMemoryBarrier memory_barrier{};
                memory_barrier.old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                memory_barrier.new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                memory_barrier.src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memory_barrier.dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
                memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                command_buffer.CreateImageMemoryBarrier(image.GetVkImageView(), memory_barrier);
            }
        }

        inline std::vector<uint8_t> GetAttributeData(const tinygltf::Model *model, uint32_t accessorId)
        {
            auto &accessor = model->accessors.at(accessorId);
            auto &bufferView = model->bufferViews.at(accessor.bufferView);
            auto &buffer = model->buffers.at(bufferView.buffer);

            size_t stride = accessor.ByteStride(bufferView);
            size_t startByte = accessor.byteOffset + bufferView.byteOffset;
            size_t endByte = startByte + accessor.count * stride;

            return {buffer.data.begin() + startByte, buffer.data.begin() + endByte};
        };

        inline VkFormat GetAttributeFormat(const tinygltf::Model *model, uint32_t accessorId)
        {
            auto &accessor = model->accessors.at(accessorId);

            VkFormat format;

            switch (accessor.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_BYTE:
            {
                static const std::map<int, VkFormat> mapped_format = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
                                                                      {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
                                                                      {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
                                                                      {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}};

                format = mapped_format.at(accessor.type);

                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            {
                static const std::map<int, VkFormat> mapped_format = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UINT},
                                                                      {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UINT},
                                                                      {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UINT},
                                                                      {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UINT}};

                static const std::map<int, VkFormat> mapped_format_normalize = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_UNORM},
                                                                                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_UNORM},
                                                                                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_UNORM},
                                                                                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UNORM}};

                if (accessor.normalized)
                {
                    format = mapped_format_normalize.at(accessor.type);
                }
                else
                {
                    format = mapped_format.at(accessor.type);
                }

                break;
            }
            case TINYGLTF_COMPONENT_TYPE_SHORT:
            {
                static const std::map<int, VkFormat> mapped_format = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R8_SINT},
                                                                      {TINYGLTF_TYPE_VEC2, VK_FORMAT_R8G8_SINT},
                                                                      {TINYGLTF_TYPE_VEC3, VK_FORMAT_R8G8B8_SINT},
                                                                      {TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}};

                format = mapped_format.at(accessor.type);

                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            {
                static const std::map<int, VkFormat> mapped_format = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UINT},
                                                                      {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UINT},
                                                                      {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UINT},
                                                                      {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UINT}};

                static const std::map<int, VkFormat> mapped_format_normalize = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R16_UNORM},
                                                                                {TINYGLTF_TYPE_VEC2, VK_FORMAT_R16G16_UNORM},
                                                                                {TINYGLTF_TYPE_VEC3, VK_FORMAT_R16G16B16_UNORM},
                                                                                {TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UNORM}};

                if (accessor.normalized)
                {
                    format = mapped_format_normalize.at(accessor.type);
                }
                else
                {
                    format = mapped_format.at(accessor.type);
                }

                break;
            }
            case TINYGLTF_COMPONENT_TYPE_INT:
            {
                static const std::map<int, VkFormat> mapped_format = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SINT},
                                                                      {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SINT},
                                                                      {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SINT},
                                                                      {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SINT}};

                format = mapped_format.at(accessor.type);

                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            {
                static const std::map<int, VkFormat> mapped_format = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_UINT},
                                                                      {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_UINT},
                                                                      {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_UINT},
                                                                      {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_UINT}};

                format = mapped_format.at(accessor.type);

                break;
            }
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
            {
                static const std::map<int, VkFormat> mapped_format = {{TINYGLTF_TYPE_SCALAR, VK_FORMAT_R32_SFLOAT},
                                                                      {TINYGLTF_TYPE_VEC2, VK_FORMAT_R32G32_SFLOAT},
                                                                      {TINYGLTF_TYPE_VEC3, VK_FORMAT_R32G32B32_SFLOAT},
                                                                      {TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SFLOAT}};

                format = mapped_format.at(accessor.type);

                break;
            }
            default:
            {
                format = VK_FORMAT_UNDEFINED;
                break;
            }
            }

            return format;
        };

        inline std::vector<uint8_t> ConvertUnderlyingDataStride(const std::vector<uint8_t> &src_data, uint32_t src_stride, uint32_t dst_stride)
        {
            auto elem_count = ToUint32_t(src_data.size()) / src_stride;

            std::vector<uint8_t> result(elem_count * dst_stride);

            for (uint32_t idxSrc = 0, idxDst = 0;
                 idxSrc < src_data.size() && idxDst < result.size();
                 idxSrc += src_stride, idxDst += dst_stride)
            {
                std::copy(src_data.begin() + idxSrc, src_data.begin() + idxSrc + src_stride, result.begin() + idxDst);
            }

            return result;
        }

        inline size_t GetAttributeSize(const tinygltf::Model *model, uint32_t accessorId)
        {
            return model->accessors.at(accessorId).count;
        };

        inline size_t GetAttributeStride(const tinygltf::Model *model, uint32_t accessorId)
        {
            auto &accessor = model->accessors.at(accessorId);
            auto &bufferView = model->bufferViews.at(accessor.bufferView);

            return accessor.ByteStride(bufferView);
        };

        inline tinygltf::Value *GetExtension(tinygltf::ExtensionMap &tinygltf_extensions, const std::string &extension)
        {
            auto it = tinygltf_extensions.find(extension);
            if (it != tinygltf_extensions.end())
                return &it->second;

            else
                return nullptr;
        }
    }

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

        bool importResult = gltf_loader.LoadASCIIFromFile(&m_Model, &err, &warn, gltf_file.generic_string());

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

        m_ModelPath = fs::path::Get(fs::path::Type::Assets, file_name).parent_path();

        LoadScene(scene_index);
        return std::move(m_Scene);
    }

    void GLTFLoader::LoadScene(int scene_index)
    {
        CheckExtensions();
        LoadScenes(scene_index);
        LoadLights();
        LoadSamplers();
        LoadImages();
        LoadTextures();
        LoadMaterials();
        LoadMeshes();
        LoadCameras();
        LoadNodes();
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

    void GLTFLoader::LoadLights()
    {
        auto light_components = ParseKHRLightsPunctual();
        m_Scene->GetLights() = std::move(light_components);
    }

    void GLTFLoader::LoadSamplers()
    {
        m_Scene->GetSamplers().resize(m_Model.samplers.size());

        for (size_t sampler_index = 0; sampler_index < m_Model.samplers.size(); sampler_index++)
        {
            auto sampler = ParseSampler(m_Model.samplers.at(sampler_index));
            m_Scene->GetSamplers()[sampler_index] = std::move(sampler);
        }
    }

    void GLTFLoader::LoadImages()
    {
        Timer timer;
        timer.Start();

        auto thread_count = std::thread::hardware_concurrency();

        // TODO: have central thread pool
        thread_count = thread_count == 0 ? 1 : thread_count;
        // thread_count = 1;
        ThreadPool thread_pool(thread_count);

        auto image_count = ToUint32_t(m_Model.images.size());

        std::vector<std::future<std::unique_ptr<sg::Image>>> image_component_futures;
        for (size_t image_index = 0; image_index < image_count; image_index++)
        {
            auto fut = thread_pool.enqueue(
                [this, image_index]()
                {
                    auto image = ParseImage(m_Model.images.at(image_index));

                    ENG_CORE_TRACE("Loaded gltf image #{} ({})", image_index, m_Model.images.at(image_index).uri.c_str());

                    return image;
                });

            image_component_futures.push_back(std::move(fut));
        }

        for (auto &fut : image_component_futures)
        {
            m_Scene->GetImages().push_back(fut.get());
        }

        auto &command_buffer = m_Device.RequestCommandBuffer();

        command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0);

        for (size_t image_index = 0; image_index < image_count; image_index++)
        {
            auto &image = m_Scene->GetImages().at(image_index);

            core::Buffer stage_buffer{m_Device,
                                      image->GetData().size(),
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      VMA_MEMORY_USAGE_CPU_ONLY};

            stage_buffer.Update(image->GetData());

            UploadImageToGpu(command_buffer, stage_buffer, *image);

            m_TransientBuffers.push_back(std::move(stage_buffer));
        }

        command_buffer.End();

        auto &queue = m_Device.GetQueueFamilyByFlags(VK_QUEUE_GRAPHICS_BIT);

        queue.GetQueues()[0].Submit(command_buffer, m_Device.RequestFence());

        m_Device.GetFencePool().Wait();
        m_Device.GetFencePool().Reset();
        m_Device.GetCommandPool().ResetPool();
        m_Device.WaitIdle();

        m_TransientBuffers.clear();

        auto elapsed_time = timer.Stop();

        ENG_CORE_INFO("Time spent loading images: {} seconds across {} threads.", engine::ToString(elapsed_time), thread_count);
    }

    void GLTFLoader::LoadTextures()
    {
        auto default_sampler = CreateDefaultSampler();

        for (auto &gltf_texture : m_Model.textures)
        {
            auto texture = ParseTexture(gltf_texture);

            texture->SetImage(*m_Scene->GetImages().at(gltf_texture.source));

            if (gltf_texture.sampler >= 0 && gltf_texture.sampler < static_cast<int>(m_Scene->GetSamplers().size()))
            {
                texture->SetSampler(*m_Scene->GetSamplers().at(gltf_texture.sampler));
            }
            else
            {
                if (gltf_texture.name.empty())
                {
                    gltf_texture.name = m_Scene->GetImages().at(gltf_texture.source)->GetName();
                }

                texture->SetSampler(*default_sampler);
            }

            m_Scene->GetTextures().emplace_back(std::move(texture));
        }

        m_Scene->GetSamplers().emplace_back(std::move(default_sampler));
    }

    void GLTFLoader::LoadMaterials()
    {
        for (auto &gltf_material : m_Model.materials)
        {
            auto material = ParseMaterial(gltf_material);

            for (auto &gltf_value : gltf_material.values)
            {
                if (gltf_value.first.find("Texture") != std::string::npos)
                {
                    std::string tex_name = ToSnakeCase(gltf_value.first);

                    material->m_Textures[tex_name] = m_Scene->GetTextures().at(gltf_value.second.TextureIndex()).get();
                }
            }

            for (auto &gltf_value : gltf_material.additionalValues)
            {
                if (gltf_value.first.find("Texture") != std::string::npos)
                {
                    std::string tex_name = ToSnakeCase(gltf_value.first);

                    material->m_Textures[tex_name] = m_Scene->GetTextures().at(gltf_value.second.TextureIndex()).get();
                }
            }

            m_Scene->GetMaterials().emplace_back(std::move(material));
        }
    }

    void GLTFLoader::LoadMeshes()
    {
        auto default_material = CreateDefaultMaterial();

        for (auto &gltf_mesh : m_Model.meshes)
        {
            auto entity = ParseMesh(gltf_mesh);
            m_Scene->GetMeshes().emplace_back(std::make_unique<Entity>(entity));

            auto &mesh = entity.GetComponent<sg::Mesh>();

            for (auto &gltf_primitive : gltf_mesh.primitives)
            {
                auto submesh = std::make_unique<sg::Submesh>();

                for (auto &attribute : gltf_primitive.attributes)
                {
                    std::string attrib_name = attribute.first;
                    std::transform(attrib_name.begin(), attrib_name.end(), attrib_name.begin(), ::tolower);

                    auto vertex_data = GetAttributeData(&m_Model, attribute.second);

                    if (attrib_name == "position")
                    {
                        submesh->m_VerticesCount = ToUint32_t(m_Model.accessors.at(attribute.second).count);
                    }

                    core::Buffer buffer{m_Device,
                                        vertex_data.size(),
                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                        VMA_MEMORY_USAGE_GPU_TO_CPU};
                    buffer.Update(vertex_data);

                    submesh->m_VertexBuffers.insert(std::make_pair(attrib_name, std::move(buffer)));

                    sg::VertexAttribute attrib;
                    attrib.format = GetAttributeFormat(&m_Model, attribute.second);
                    attrib.stride = ToUint32_t(GetAttributeStride(&m_Model, attribute.second));

                    submesh->SetAttribute(attrib_name, attrib);
                }

                if (gltf_primitive.indices >= 0)
                {
                    submesh->m_VertexIndices = ToUint32_t(GetAttributeSize(&m_Model, gltf_primitive.indices));

                    auto format = GetAttributeFormat(&m_Model, gltf_primitive.indices);

                    auto vertex_data = GetAttributeData(&m_Model, gltf_primitive.indices);
                    auto index_data = GetAttributeData(&m_Model, gltf_primitive.indices);

                    switch (format)
                    {
                    case VK_FORMAT_R8_UINT:
                        // Converts uint8 data into uint16 data, still represented by a uint8 vector
                        index_data = ConvertUnderlyingDataStride(index_data, 1, 2);
                        submesh->m_IndexType = VK_INDEX_TYPE_UINT16;
                        break;
                    case VK_FORMAT_R16_UINT:
                        submesh->m_IndexType = VK_INDEX_TYPE_UINT16;
                        break;
                    case VK_FORMAT_R32_UINT:
                        submesh->m_IndexType = VK_INDEX_TYPE_UINT32;
                        break;
                    default:
                        ENG_CORE_ERROR("gltf primitive has invalid format type");
                        break;
                    }

                    submesh->m_IndexBuffer = std::make_unique<core::Buffer>(m_Device,
                                                                            index_data.size(),
                                                                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                                            VMA_MEMORY_USAGE_GPU_TO_CPU);

                    submesh->m_IndexBuffer->Update(index_data);
                }
                else
                {
                    submesh->m_VerticesCount = ToUint32_t(GetAttributeSize(&m_Model, gltf_primitive.attributes.at("POSITION")));
                }

                if (gltf_primitive.material < 0)
                {
                    submesh->SetMaterial(*default_material);
                }
                else
                {
                    submesh->SetMaterial(*m_Scene->GetMaterials().at(gltf_primitive.material));
                }

                m_Scene->GetSubmeshes().emplace_back(std::move(submesh));
                mesh.AddSubmesh(*m_Scene->GetSubmeshes().back());
            }
        }

        m_Device.GetFencePool().Wait();
        m_Device.GetFencePool().Reset();
        m_Device.GetCommandPool().ResetPool();

        m_TransientBuffers.clear();
    }

    void GLTFLoader::LoadCameras()
    {
        for (auto &gltf_camera : m_Model.cameras)
        {
            auto entity = ParseCamera(gltf_camera);
            m_Scene->GetCameras().emplace_back(std::make_unique<Entity>(entity));
        }
    }

    void GLTFLoader::LoadAnimations()
    {
    }

    void GLTFLoader::LoadScenes(int scene_index)
    {
        tinygltf::Scene *gltf_scene{nullptr};
        if (scene_index >= 0 && scene_index < static_cast<int>(m_Model.scenes.size()))
            gltf_scene = &m_Model.scenes[scene_index];

        else if (m_Model.defaultScene >= 0 && m_Model.defaultScene < static_cast<int>(m_Model.scenes.size()))
            gltf_scene = &m_Model.scenes[m_Model.defaultScene];

        else if (m_Model.scenes.size() > 0)
            gltf_scene = &m_Model.scenes[0];

        if (!gltf_scene)
            throw std::runtime_error("Couldn't determine which scene to load!");

        m_Scene = std::make_unique<Scene>(gltf_scene->name);
    }

    void GLTFLoader::LoadNodes()
    {
        for (size_t node_index = 0; node_index < m_Model.nodes.size(); ++node_index)
        {
            auto gltf_node = m_Model.nodes[node_index];

            Entity entity;
            bool identified = false;

            if (gltf_node.mesh >= 0)
            {
                if (gltf_node.mesh == 361)
                    ENG_CORE_ERROR("fuck");
                entity = *m_Scene->GetMeshes().at(gltf_node.mesh);
                identified = true;
            }

            if (gltf_node.camera >= 0)
            {
                entity = *m_Scene->GetCameras().at(gltf_node.camera);
                identified = true;
            }

            if (auto extension = GetExtension(gltf_node.extensions, KHR_LIGHTS_PUNCTUAL_EXTENSION))
            {
                auto index = extension->Get("light").Get<int>();
                entity = *m_Scene->GetLights().at(index).get();
                identified = true;
            }

            if (!identified)
            {
                entity = m_Scene->CreateEntity();
            }

            ParseNode(gltf_node, entity);

            ENG_ASSERT(entity.HasComponent<sg::Transform>());
        }
    }

    std::vector<std::unique_ptr<Entity>> GLTFLoader::ParseKHRLightsPunctual()
    {
        if (IsExtensionEnabled(KHR_LIGHTS_PUNCTUAL_EXTENSION))
        {
            if (m_Model.extensions.find(KHR_LIGHTS_PUNCTUAL_EXTENSION) == m_Model.extensions.end() || !m_Model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Has("lights"))
                return {};

            auto &khr_lights = m_Model.extensions.at(KHR_LIGHTS_PUNCTUAL_EXTENSION).Get("lights");

            std::vector<std::unique_ptr<Entity>> light_components(khr_lights.ArrayLen());

            for (size_t light_index = 0; light_index < khr_lights.ArrayLen(); ++light_index)
            {
                auto &khr_light = khr_lights.Get(static_cast<int>(light_index));

                // Spec states a light has to have a type to be valid
                if (!khr_light.Has("type"))
                {
                    ENG_CORE_ERROR("KHR_lights_punctual extension: light {} doesn't have a type!", light_index);
                    throw std::runtime_error("Couldn't load glTF file, KHR_lights_punctual extension is invalid");
                }

                // TODO: entity moves
                light_components[light_index] = std::make_unique<Entity>(
                    m_Scene->CreateEntity());

                auto &entity = *light_components[light_index].get();

                auto &light = entity.AddComponent<sg::Light>(khr_light.Get("name").Get<std::string>(),
                                                             entity);

                sg::LightType type;
                sg::LightProperties properties;

                // Get type
                auto &gltf_light_type = khr_light.Get("type").Get<std::string>();
                if (gltf_light_type == "point")
                {
                    type = sg::LightType::Point;
                }
                else if (gltf_light_type == "spot")
                {
                    type = sg::LightType::Spot;
                }
                else if (gltf_light_type == "directional")
                {
                    type = sg::LightType::Directional;
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

                if (type != sg::LightType::Directional)
                {
                    properties.range = static_cast<float>(khr_light.Get("range").Get<double>());
                    if (type != sg::LightType::Point)
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
                else if (type == sg::LightType::Directional || type == sg::LightType::Spot)
                {
                    // The spec states that the light will inherit the transform of the node.
                    // The light's direction is defined as the 3-vector (0.0, 0.0, -1.0) and
                    // the rotation of the node orients the light accordingly.
                    properties.direction = glm::vec3(0.0f, 0.0f, -1.0f);
                }

                light.SetType(type);
                light.SetProperties(properties);
            }

            return light_components;
        }

        else
        {
            return {};
        }
    }

    std::unique_ptr<sg::Sampler> GLTFLoader::ParseSampler(const tinygltf::Sampler &gltf_sampler) const
    {
        auto name = gltf_sampler.name;

        VkFilter min_filter = FindMinFilter(gltf_sampler.minFilter);
        VkFilter mag_filter = FindMagFilter(gltf_sampler.magFilter);

        VkSamplerMipmapMode mipmap_mode = FindMipmapMode(gltf_sampler.minFilter);

        VkSamplerAddressMode address_mode_u = FindWrapMode(gltf_sampler.wrapS);
        VkSamplerAddressMode address_mode_v = FindWrapMode(gltf_sampler.wrapT);
        // TODO: tinygltf incompatible
        VkSamplerAddressMode address_mode_w = FindWrapMode(TINYGLTF_TEXTURE_WRAP_REPEAT);

        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = mag_filter;
        sampler_info.minFilter = min_filter;
        sampler_info.mipmapMode = mipmap_mode;
        sampler_info.addressModeU = address_mode_u;
        sampler_info.addressModeV = address_mode_v;
        sampler_info.addressModeW = address_mode_w;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampler_info.maxLod = std::numeric_limits<float>::max();

        core::Sampler vk_sampler{m_Device, sampler_info};

        return std::make_unique<sg::Sampler>(name, std::move(vk_sampler));
    }

    std::unique_ptr<sg::Image> GLTFLoader::ParseImage(tinygltf::Image &gltf_image) const
    {
        std::unique_ptr<sg::Image> image{nullptr};
        auto image_uri = m_ModelPath / gltf_image.uri;

        if (!gltf_image.image.empty())
        {
            // Image embedded in gltf file
            auto mipmap = sg::Mipmap{0, 0, {static_cast<uint32_t>(gltf_image.width), static_cast<uint32_t>(gltf_image.height), 1u}};

            std::vector<sg::Mipmap> mipmaps{mipmap};
            image = std::make_unique<sg::Image>(gltf_image.name, std::move(gltf_image.image), std::move(mipmaps));
        }
        else
        {
            // Load image from uri
            image = sg::Image::Load(gltf_image.name, image_uri);
        }

        // TODO: astc old commit
        // Check whether the format is supported by the GPU
        if (sg::IsAstc(image->GetFormat()))
        {
            if (!m_Device.IsImageFormatSupported(image->GetFormat()))
            {
                ENG_CORE_WARN("ASTC not supported: decoding {}", image_uri.generic_string());
                image = std::make_unique<sg::Astc>(*image);
                image->GenerateMipmaps();
            }
        }

        image->CreateVkImage(m_Device);

        return image;
    }

    std::unique_ptr<sg::Texture> GLTFLoader::ParseTexture(const tinygltf::Texture &gltf_texture) const
    {
        return std::make_unique<sg::Texture>(gltf_texture.name);
    }

    bool GLTFLoader::IsExtensionEnabled(const std::string &requested_extension)
    {
        auto it = m_SupportedExtensions.find(requested_extension);

        if (it != m_SupportedExtensions.end())
            return it->second;
        else
            return false;
    }

    std::unique_ptr<sg::PBRMaterial> GLTFLoader::ParseMaterial(const tinygltf::Material &gltf_material) const
    {
        auto material = std::make_unique<sg::PBRMaterial>(gltf_material.name);

        for (auto &gltf_value : gltf_material.values)
        {
            if (gltf_value.first == "baseColorFactor")
            {
                const auto &color_factor = gltf_value.second.ColorFactor();
                material->m_BaseColorFactor = glm::vec4(color_factor[0], color_factor[1], color_factor[2], color_factor[3]);
            }
            else if (gltf_value.first == "metallicFactor")
            {
                material->m_MetallicFactor = static_cast<float>(gltf_value.second.Factor());
            }
            else if (gltf_value.first == "roughnessFactor")
            {
                material->m_RoughnessFactor = static_cast<float>(gltf_value.second.Factor());
            }
        }

        for (auto &gltf_value : gltf_material.additionalValues)
        {
            if (gltf_value.first == "emissiveFactor")
            {
                const auto &emissive_factor = gltf_value.second.number_array;

                material->m_Emissive = glm::vec3(emissive_factor[0], emissive_factor[1], emissive_factor[2]);
            }
            else if (gltf_value.first == "alphaMode")
            {
                if (gltf_value.second.string_value == "BLEND")
                {
                    material->m_AlphaMode = sg::AlphaMode::Blend;
                }
                else if (gltf_value.second.string_value == "OPAQUE")
                {
                    material->m_AlphaMode = sg::AlphaMode::Opaque;
                }
                else if (gltf_value.second.string_value == "MASK")
                {
                    material->m_AlphaMode = sg::AlphaMode::Mask;
                }
            }
            else if (gltf_value.first == "alphaCutoff")
            {
                material->m_AlphaCutoff = static_cast<float>(gltf_value.second.number_value);
            }
            else if (gltf_value.first == "doubleSided")
            {
                material->m_DoubleSided = gltf_value.second.bool_value;
            }
        }

        return material;
    }

    Entity GLTFLoader::ParseMesh(const tinygltf::Mesh &gltf_mesh) const
    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<sg::Mesh>(gltf_mesh.name);
        return entity;
    }

    Entity GLTFLoader::ParseCamera(const tinygltf::Camera &gltf_camera) const
    {
        auto entity = m_Scene->CreateEntity();
        if (gltf_camera.type == "perspective")
        {
            auto &camera = entity.AddComponent<sg::PerspectiveCamera>(gltf_camera.name);

            camera.m_AspectRatio = static_cast<float>(gltf_camera.perspective.aspectRatio);
            camera.m_Fov = static_cast<float>(gltf_camera.perspective.yfov);
            camera.m_NearPlane = static_cast<float>(gltf_camera.perspective.znear);
            camera.m_FarPlane = static_cast<float>(gltf_camera.perspective.zfar);
        }

        else
            ENG_CORE_WARN("Camera type not supported");

        return entity;
    }

    void GLTFLoader::ParseNode(tinygltf::Node &gltf_node, Entity &entity)
    {
        if (entity.HasComponent<sg::Transform>())
            return;

        auto &transform = entity.AddComponent<sg::Transform>(entity);
        if (!gltf_node.translation.empty())
        {
            glm::vec3 translation;

            std::transform(gltf_node.translation.begin(),
                           gltf_node.translation.end(),
                           glm::value_ptr(translation),
                           TypeCast<double, float>{});

            ENG_CORE_INFO(glm::to_string(translation));

            transform.SetTranslation(translation);
        }

        if (!gltf_node.rotation.empty())
        {
            glm::quat rotation;

            std::transform(gltf_node.rotation.begin(),
                           gltf_node.rotation.end(),
                           glm::value_ptr(rotation),
                           TypeCast<double, float>{});

            ENG_CORE_INFO(glm::to_string(rotation));

            transform.SetRotation(rotation);
        }

        if (!gltf_node.scale.empty())
        {
            glm::vec3 scale;

            std::transform(gltf_node.scale.begin(),
                           gltf_node.scale.end(),
                           glm::value_ptr(scale),
                           TypeCast<double, float>{});

            ENG_CORE_INFO(glm::to_string(scale));

            transform.SetScale(scale);
        }

        if (!gltf_node.matrix.empty())
        {
            glm::mat4 matrix;

            std::transform(gltf_node.matrix.begin(),
                           gltf_node.matrix.end(),
                           glm::value_ptr(matrix),
                           TypeCast<double, float>{});

            ENG_CORE_INFO(glm::to_string(matrix));

            transform.SetMatrix(matrix);
        }
    }

    std::unique_ptr<sg::Sampler> GLTFLoader::CreateDefaultSampler()
    {
        tinygltf::Sampler gltf_sampler;

        gltf_sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
        gltf_sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;

        gltf_sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
        gltf_sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;
        //TODO: sampler wrapR
        // gltf_sampler.wrapR = TINYGLTF_TEXTURE_WRAP_REPEAT;

        return ParseSampler(gltf_sampler);
    }

    std::unique_ptr<sg::PBRMaterial> GLTFLoader::CreateDefaultMaterial()
    {
        tinygltf::Material gltf_material;
        return ParseMaterial(gltf_material);
    }
}
