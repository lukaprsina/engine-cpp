#include "scene/components/image.h"

#include "platform/filesystem.h"
#include "scene/components/image/ktx.h"
#include "scene/components/image/stb.h"
#include "vulkan_api/core/image.h"
#include "vulkan_api/core/image_view.h"

ENG_DISABLE_WARNINGS()
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace sg
    {
        bool IsAstc(const VkFormat format)
        {
            return (format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_4x4_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_5x4_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_5x4_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_5x5_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_5x5_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_6x5_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_6x5_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_6x6_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_6x6_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_8x5_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_8x5_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_8x6_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_8x6_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_8x8_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_8x8_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_10x5_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_10x5_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_10x6_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_10x6_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_10x8_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_10x8_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_10x10_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_10x10_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_12x10_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_12x10_SRGB_BLOCK ||
                    format == VK_FORMAT_ASTC_12x12_UNORM_BLOCK ||
                    format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK);
        }

        Image::Image(const std::string &name, std::vector<uint8_t> &&data, std::vector<Mipmap> &&mipmaps)
            : m_Name{name},
              m_Data{std::move(data)},
              m_Format{VK_FORMAT_R8G8B8A8_UNORM},
              m_Mipmaps{std::move(mipmaps)}
        {
        }

        Image::~Image()
        {
        }

        std::unique_ptr<Image> Image::Load(const std::string &name, const std::filesystem::path path)
        {
            std::unique_ptr<Image> image{};

            auto data = fs::ReadBinaryFile(fs::path::Get(fs::path::Type::Assets, path.generic_string()));
            auto extension = path.extension().generic_string();

            if (extension == ".png" || extension == ".jpg")
            {
                image = std::make_unique<Stb>(name, data);
            }
            else if (extension == ".ktx")
            {
                image = std::make_unique<Ktx>(name, data);
            }
            else if (extension == ".ktx2")
            {
                image = std::make_unique<Ktx>(name, data);
            }

            return image;
        }

        void Image::GenerateMipmaps()
        {
            assert(m_Mipmaps.size() == 1 && "Mipmaps already present");

            if (m_Mipmaps.size() > 1)
                return;

            auto extent = GetExtent();
            auto next_width = std::max<uint32_t>(1u, extent.width / 2);
            auto next_height = std::max<uint32_t>(1u, extent.height / 2);
            auto channels = 4;
            auto next_size = next_width * next_height * channels;

            while (true)
            {
                auto old_size = ToUint32_t(m_Data.size());
                m_Data.resize(old_size + next_size);
                auto &previous_mipmap = m_Mipmaps.back();

                Mipmap next_mipmap{};
                next_mipmap.level = previous_mipmap.level + 1;
                next_mipmap.offset = old_size;
                next_mipmap.extent = {next_width, next_height, 1u};

                stbir_resize_uint8(m_Data.data() + previous_mipmap.offset,
                                   previous_mipmap.extent.width,
                                   previous_mipmap.extent.height, 0,

                                   m_Data.data() + next_mipmap.offset,
                                   next_mipmap.extent.width,
                                   next_mipmap.extent.height, 0, channels);

                m_Mipmaps.emplace_back(std::move(next_mipmap));

                next_width = std::max<uint32_t>(1u, next_width / 2);
                next_height = std::max<uint32_t>(1u, next_height / 2);
                next_size = next_width * next_height * channels;

                if (next_width == 1 && next_height == 1)
                {
                    break;
                }
            }
        }

        void Image::CreateVkImage(Device &device, VkImageViewType image_view_type, VkImageCreateFlags flags)
        {
            assert(!m_VkImage && !m_VkImageView && "Vulkan image already constructed");

            m_VkImage = std::make_unique<core::Image>(device,
                                                      GetExtent(),
                                                      m_Format,
                                                      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                      VMA_MEMORY_USAGE_GPU_ONLY,
                                                      VK_SAMPLE_COUNT_1_BIT,
                                                      ToUint32_t(m_Mipmaps.size()),
                                                      m_Layers,
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      flags);

            m_VkImageView = std::make_unique<core::ImageView>(*m_VkImage, image_view_type);
        }

        void Image::SetData(const uint8_t *raw_data, size_t size)
        {
            ENG_ASSERT(m_Data.empty() && "Image data already set");
            m_Data = {raw_data, raw_data + size};
        }

        void Image::ClearData()
        {
            m_Data.clear();
            m_Data.shrink_to_fit();
        }

        const core::Image &Image::GetVkImage() const
        {
            ENG_ASSERT(m_VkImage && "Vulkan image was not created");
            return *m_VkImage;
        }

        const core::ImageView &Image::GetVkImageView() const
        {
            ENG_ASSERT(m_VkImageView && "Vulkan image view was not created");
            return *m_VkImageView;
        }
    }
}
