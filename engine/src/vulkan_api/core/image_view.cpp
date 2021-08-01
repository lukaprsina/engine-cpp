#include "vulkan_api/core/image_view.h"

#include "vulkan_api/device.h"

namespace engine
{
    namespace core
    {
        ImageView::ImageView(Image &image, VkImageViewType view_type, VkFormat format,
                             uint32_t mip_level, uint32_t array_layer,
                             uint32_t n_mip_levels, uint32_t n_array_layers)
            : m_Device(image.GetDevice()),
              m_Image(&image),
              m_Format(format)
        {
            if (format == VK_FORMAT_UNDEFINED)
            {
                m_Format = m_Image->GetFormat();
                format = m_Format;
            }

            m_SubresourceRange.baseMipLevel = mip_level;
            m_SubresourceRange.baseArrayLayer = array_layer;
            m_SubresourceRange.levelCount = n_mip_levels == 0 ? m_Image->GetSubresource().mipLevel : n_mip_levels;
            m_SubresourceRange.layerCount = n_array_layers == 0 ? m_Image->GetSubresource().arrayLayer : n_array_layers;

            if (IsDepthStencilFormat(format))
                m_SubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            else
                m_SubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view_info.image = m_Image->GetHandle();
            view_info.viewType = view_type;
            view_info.format = format;
            view_info.subresourceRange = m_SubresourceRange;

            auto result = vkCreateImageView(m_Device.GetHandle(), &view_info, nullptr, &m_Handle);

            if (result != VK_SUCCESS)
            {
                throw VulkanException{result, "Cannot create ImageView"};
            }

            // Register this image view to its image
            // in order to be notified when it gets moved
            m_Image->GetViews().emplace(this);
        }

        ImageView::ImageView(ImageView &&other)
            : m_Device(other.m_Device),
              m_Image(other.m_Image),
              m_Format(other.m_Format),
              m_Handle(other.m_Handle),
              m_SubresourceRange(other.m_SubresourceRange)
        {
            auto &views = m_Image->GetViews();
            views.erase(&other);
            views.emplace(this);

            other.m_Handle = VK_NULL_HANDLE;
        }

        ImageView::~ImageView()
        {
            if (m_Handle != VK_NULL_HANDLE)
                vkDestroyImageView(m_Device.GetHandle(), m_Handle, nullptr);
        }
    }
}
