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
            Image(const std::string &name, std::vector<uint8_t> &&data = {}, std::vector<Mipmap> &&mipmaps = {{}});
            virtual ~Image();

            static std::unique_ptr<Image> Load(const std::string &name, const std::filesystem::path path);
            void GenerateMipmaps();
            void CreateVkImage(Device &device, VkImageViewType image_view_type = VK_IMAGE_VIEW_TYPE_2D, VkImageCreateFlags flags = 0);

            void ClearData();
            const std::string &GetName() const { return m_Name; }
            VkFormat GetFormat() const { return m_Format; }
            const VkExtent3D &GetExtent() const { return m_Mipmaps.at(0).extent; }
            const std::vector<uint8_t> &GetData() const { return m_Data; }
            const core::Image &GetVkImage() const;
            const core::ImageView &GetVkImageView() const;
            const std::vector<Mipmap> &GetMipmaps() const { return m_Mipmaps; }

        protected:
            std::vector<uint8_t> &GetMutData() { return m_Data; }
            void SetData(const uint8_t *raw_data, size_t size);
            void SetFormat(VkFormat format) { m_Format = format; }
            void SetWidth(uint32_t width) { m_Mipmaps.at(0).extent.width = width; }
            void SetHeight(uint32_t height) { m_Mipmaps.at(0).extent.height = height; }
            void SetDepth(uint32_t depth) { m_Mipmaps.at(0).extent.depth = depth; }
            void SetLayers(uint32_t layers) { m_Layers = layers; }
            void SetOffsets(const std::vector<std::vector<VkDeviceSize>> &offsets) { m_Offsets = offsets; }
            Mipmap &GetMipmap(size_t index) { return m_Mipmaps.at(index); }
            std::vector<Mipmap> &GetMutMipmaps() { return m_Mipmaps; }

        private:
            std::string m_Name;
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
