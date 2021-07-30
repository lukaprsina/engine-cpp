#pragma once

#include <unordered_set>

namespace engine
{
    class Device;

    namespace core
    {
        class ImageView;
        class Image
        {
        public:
            Image(Device &device,
                  VkImage handle,
                  const VkExtent3D &extent,
                  VkFormat format,
                  VkImageUsageFlags image_usage,
                  VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);

            Image(Device &device,
                  const VkExtent3D &extent,
                  VkFormat format,
                  VkImageUsageFlags image_usage,
                  VmaMemoryUsage memory_usage,
                  VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
                  uint32_t mip_levels = 1,
                  uint32_t array_layers = 1,
                  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                  VkImageCreateFlags flags = 0,
                  uint32_t num_queue_families = 0,
                  const uint32_t *queue_families = nullptr);

            ~Image();

            Device &GetDevice() { return m_Device; }
            VkImage &GetHandle() { return m_Handle; }
            const VkExtent3D &GetExtent() const { return m_Extent; }
            VkImageType GetType() const { return m_Type; }
            VkFormat GetFormat() const { return m_Format; }
            VkImageUsageFlags GetUsage() const { return m_Usage; }
            VkSampleCountFlagBits GetSampleCount() const { return m_SampleCount; }
            VkImageSubresource GetSubresource() const { return m_Subresource; }
            std::unordered_set<ImageView *> &GetViews() { return m_ImageViews; }

        private:
            Device &m_Device;
            VkImage m_Handle;
            VmaAllocation m_Memory;
            VkImageType m_Type;
            VkExtent3D m_Extent;
            VkFormat m_Format;
            VkImageUsageFlags m_Usage;
            VkSampleCountFlagBits m_SampleCount;
            VkImageTiling m_Tiling;
            VkImageSubresource m_Subresource;
            uint32_t m_ArrayLayerCount;

            /// Image views referring to this image
            std::unordered_set<ImageView *> m_ImageViews;
            uint8_t *m_MappedData{nullptr};

            /// Whether it was mapped with vmaMapMemory
            bool mapped{false};
        };
    }
}