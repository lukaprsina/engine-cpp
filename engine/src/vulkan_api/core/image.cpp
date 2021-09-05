#include "vulkan_api/core/image.h"

#include "vulkan_api/device.h"
#include "vulkan_api/core/image_view.h"

namespace engine
{
    namespace
    {
        VkImageType FindImageType(VkExtent3D extent)
        {
            VkImageType result{};

            uint32_t dim_num{0};

            if (extent.width >= 1)
            {
                dim_num++;
            }

            if (extent.height >= 1)
            {
                dim_num++;
            }

            if (extent.depth > 1)
            {
                dim_num++;
            }

            switch (dim_num)
            {
            case 1:
                result = VK_IMAGE_TYPE_1D;
                break;
            case 2:
                result = VK_IMAGE_TYPE_2D;
                break;
            case 3:
                result = VK_IMAGE_TYPE_3D;
                break;
            default:
                throw std::runtime_error("No image type found.");
                break;
            }

            return result;
        }
    }

    namespace core
    {
        Image::Image(Device &device,
                     const VkExtent3D &extent,
                     VkFormat format,
                     VkImageUsageFlags image_usage,
                     VmaMemoryUsage memory_usage,
                     VkSampleCountFlagBits sample_count,
                     const uint32_t mip_levels,
                     const uint32_t array_layers,
                     VkImageTiling tiling,
                     VkImageCreateFlags flags,
                     uint32_t num_queue_families,
                     const uint32_t *queue_families)
            : m_Device(device),
              m_Type(FindImageType(extent)),
              m_Extent(extent),
              m_Format(format),
              m_Usage(image_usage),
              m_SampleCount(sample_count),
              m_Tiling(tiling),
              m_ArrayLayerCount(array_layers)
        {
            ENG_ASSERT(mip_levels > 0, "Image should have at least one level");
            ENG_ASSERT(array_layers > 0, "Image should have at least one layer");

            m_Subresource.mipLevel = mip_levels;
            m_Subresource.arrayLayer = array_layers;

            VkImageCreateInfo image_info{};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.flags = flags;
            image_info.imageType = m_Type;
            image_info.format = format;
            image_info.extent = extent;
            image_info.mipLevels = mip_levels;
            image_info.arrayLayers = array_layers;
            image_info.samples = sample_count;
            image_info.tiling = tiling;
            image_info.usage = image_usage;

            if (num_queue_families != 0)
            {
                image_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
                image_info.queueFamilyIndexCount = num_queue_families;
                image_info.pQueueFamilyIndices = queue_families;
            }

            VmaAllocationCreateInfo memory_info{};
            memory_info.usage = memory_usage;

            if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
            {
                memory_info.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
            }

            auto result = vmaCreateImage(device.GetMemoryAllocator(),
                                         &image_info, &memory_info,
                                         &m_Handle, &m_Memory,
                                         nullptr);

            if (result != VK_SUCCESS)
            {
                throw VulkanException{result, "Cannot create Image"};
            }
        }

        Image::Image(Image &&other)
            : m_Device{other.m_Device},
              m_Handle{other.m_Handle},
              m_Memory{other.m_Memory},
              m_Type{other.m_Type},
              m_Extent{other.m_Extent},
              m_Format{other.m_Format},
              m_Usage{other.m_Usage},
              m_SampleCount{other.m_SampleCount},
              m_Tiling{other.m_Tiling},
              m_Subresource{other.m_Subresource},
              m_MappedData{other.m_MappedData},
              m_Mapped{other.m_Mapped}
        {
            other.m_Handle = VK_NULL_HANDLE;
            other.m_Memory = VK_NULL_HANDLE;
            other.m_MappedData = nullptr;
            other.m_Mapped = false;

            // Update image views references to this image to avoid dangling pointers
            for (auto &view : m_ImageViews)
            {
                view->SetImage(*this);
            }
        }

        Image::Image(Device &device, VkImage handle,
                     const VkExtent3D &extent, VkFormat format,
                     VkImageUsageFlags image_usage,
                     VkSampleCountFlagBits sample_count)
            : m_Device(device),
              m_Handle(handle),
              m_Type(FindImageType(extent)),
              m_Extent(extent),
              m_Format(format),
              m_Usage(image_usage),
              m_SampleCount(sample_count)

        {
            m_Subresource.mipLevel = 1;
            m_Subresource.arrayLayer = 1;
        }

        Image::~Image()
        {
            if (m_Handle != VK_NULL_HANDLE && m_Memory != VK_NULL_HANDLE)
            {
                Unmap();
                vmaDestroyImage(m_Device.GetMemoryAllocator(), m_Handle, m_Memory);
            }
        }

        void Image::Unmap()
        {
            if (m_Mapped)
            {
                vmaUnmapMemory(m_Device.GetMemoryAllocator(), m_Memory);
                m_MappedData = nullptr;
                m_Mapped = false;
            }
        }
    }
}