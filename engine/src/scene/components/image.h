#pragma once

namespace engine
{
    class Device;

    namespace core
    {
        class Image;
        class ImageView;
    }

    namespace sg
    {
        bool IsAstc(const VkFormat format);

        struct Mipmap
        {
            uint32_t level = 0;
            uint32_t offset = 0;
            VkExtent3D extent = {0, 0, 0};
        };

        class Image
        {
        public:
            Image();
            virtual ~Image();

            static std::unique_ptr<Image> Load(const std::string &name, const std::filesystem::path path);
            void GenerateMipmaps();
            void CreateVkImage(Device &device, VkImageViewType image_view_type = VK_IMAGE_VIEW_TYPE_2D, VkImageCreateFlags flags = 0);

            VkFormat GetFormat() const { return m_Format; }
            const VkExtent3D &GetExtent() const { m_Mipmaps.at(0).extent; }

        private:
            std::vector<uint8_t> m_Data;
            VkFormat m_Format{VK_FORMAT_UNDEFINED};
            uint32_t m_Layers{1};
            std::vector<Mipmap> m_Mipmaps{{}};
            std::vector<std::vector<VkDeviceSize>> m_Offsets;
            std::unique_ptr<core::Image> m_VkImage;
            std::unique_ptr<core::ImageView> m_VkImageView;
        };
    }
}
