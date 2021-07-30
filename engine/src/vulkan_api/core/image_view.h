#pragma once

#include "vulkan_api/core/image.h"

namespace engine
{
    namespace core
    {
        class ImageView
        {
        public:
            ImageView(Image &image, VkImageViewType view_type, VkFormat format = VK_FORMAT_UNDEFINED,
                      uint32_t base_mip_level = 0, uint32_t base_array_layer = 0,
                      uint32_t n_mip_levels = 0, uint32_t n_array_layers = 0);

            ImageView(ImageView &) = delete;

            ImageView(ImageView &&other);

            ~ImageView();

            ImageView &operator=(const ImageView &) = delete;

            ImageView &operator=(ImageView &&) = delete;

        private:
            Device &m_Device;
            Image *m_Image;
            VkFormat m_Format;
            VkImageView m_Handle;
            VkImageSubresourceRange m_SubresourceRange;
        };
    }
}
