#include "vulkan_api/device.h"

#include "vulkan_api/instance.h"
#include "vulkan_api/command_pool.h"
#include "vulkan_api/fence_pool.h"

ENG_DISABLE_WARNINGS()
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
ENG_ENABLE_WARNINGS()

namespace engine
{
    Device::Device(PhysicalDevice &gpu,
                   VkSurfaceKHR surface,
                   std::unordered_map<const char *, bool> requested_extensions)
        : m_Gpu(gpu), m_ResourceCache(*this)
    {
        ENG_CORE_INFO("Selected GPU: {}", gpu.GetProperties().deviceName);

        std::vector<VkQueueFamilyProperties> queue_family_properties = gpu.GetQueueFamilyProperties();
        uint32_t queue_family_properties_count = ToUint32_t(queue_family_properties.size());

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_family_properties_count);
        std::vector<std::vector<float>> queue_priorities(queue_family_properties_count);

        for (uint32_t queue_family_index = 0; queue_family_index < queue_family_properties_count; ++queue_family_index)
        {
            const VkQueueFamilyProperties &queue_family_property = queue_family_properties[queue_family_index];
            queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 1.0f);

            VkDeviceQueueCreateInfo &queue_create_info = queue_create_infos[queue_family_index];
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family_index;
            queue_create_info.queueCount = queue_family_property.queueCount;
            queue_create_info.pQueuePriorities = queue_priorities[queue_family_index].data();
        }

        uint32_t device_extension_count;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu.GetHandle(), nullptr, &device_extension_count, nullptr));
        m_DeviceExtensions.resize(device_extension_count);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(gpu.GetHandle(), nullptr, &device_extension_count, m_DeviceExtensions.data()));

        if (m_DeviceExtensions.size() > 0)
        {
            ENG_CORE_TRACE("Device supports the following extensions:");
            for (auto &extension : m_DeviceExtensions)
            {
                ENG_CORE_TRACE("  \t{}", extension.extensionName);
            }
        }

        if (IsExtensionSupported("VK_KHR_get_memory_requirements2") &&
            IsExtensionSupported("VK_KHR_dedicated_allocation"))
        {
            m_EnabledExtensions.push_back("VK_KHR_get_memory_requirements2");
            m_EnabledExtensions.push_back("VK_KHR_dedicated_allocation");

            ENG_CORE_INFO("Dedicated Allocation enabled");
        }

        std::vector<const char *> unsupported_extensions;
        for (auto &extension : requested_extensions)
        {
            if (IsExtensionSupported(extension.first))
                m_EnabledExtensions.emplace_back(extension.first);
            else
                unsupported_extensions.emplace_back(extension.first);
        }

        if (m_EnabledExtensions.size() > 0)
        {
            ENG_CORE_INFO("Enabled extensions:");
            for (auto &extension : m_EnabledExtensions)
            {
                ENG_CORE_INFO("  \t{}", extension);
            }
        }

        for (auto &extension : unsupported_extensions)
        {
            bool is_optional = requested_extensions[extension];

            if (is_optional)
                ENG_CORE_WARN("Optional device extension {} not available, some features may be disabled", extension);
            else
            {
                ENG_CORE_CRITICAL("Required device extension {} not available, cannot run", extension);
                throw VulkanException(VK_ERROR_EXTENSION_NOT_PRESENT, "Extensions not present");
            }
        }

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = ToUint32_t(queue_create_infos.size());
        create_info.ppEnabledExtensionNames = m_EnabledExtensions.data();
        create_info.enabledExtensionCount = ToUint32_t(m_EnabledExtensions.size());

        const auto requested_gpu_features = gpu.GetRequestedFeatures();
        create_info.pEnabledFeatures = &requested_gpu_features;

        VkResult result = vkCreateDevice(gpu.GetHandle(), &create_info, nullptr, &m_Handle);

        if (result != VK_SUCCESS)
            throw VulkanException(result, "Cannot create device");

        for (uint32_t queue_family_index = 0; queue_family_index < queue_family_properties_count; ++queue_family_index)
        {
            const VkQueueFamilyProperties &queue_family_property = queue_family_properties[queue_family_index];
            VkBool32 present_supported = gpu.IsPresentSupported(surface, queue_family_index);

            m_QueueFamilies.emplace_back(*this, queue_family_index, queue_family_property, present_supported);
        }

        VmaVulkanFunctions vma_vulkan_func{};
        vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
        vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
        vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
        vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
        vma_vulkan_func.vkCreateImage = vkCreateImage;
        vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
        vma_vulkan_func.vkDestroyImage = vkDestroyImage;
        vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vma_vulkan_func.vkFreeMemory = vkFreeMemory;
        vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vma_vulkan_func.vkMapMemory = vkMapMemory;
        vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
        vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

        VmaAllocatorCreateInfo allocator_info{};
        allocator_info.physicalDevice = gpu.GetHandle();
        allocator_info.device = m_Handle;
        allocator_info.instance = gpu.GetInstance().GetHandle();

        if (IsExtensionSupported("VK_KHR_get_memory_requirements2") &&
            IsExtensionSupported("VK_KHR_dedicated_allocation"))
        {
            allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
            vma_vulkan_func.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
            vma_vulkan_func.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
        }

        if (IsExtensionSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && IsExtensionSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
        {
            allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        }

        allocator_info.pVulkanFunctions = &vma_vulkan_func;

        result = vmaCreateAllocator(&allocator_info, &m_MemoryAllocator);

        if (result != VK_SUCCESS)
            throw VulkanException{result, "Cannot create allocator"};

        uint32_t family_index = GetQueueFamilyByFlags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT).GetFamilyIndex();

        m_CommandPool = std::make_unique<CommandPool>(*this, family_index);
        m_FencePool = std::make_unique<FencePool>(*this);
    }

    Device::~Device()
    {
        m_CommandPool.reset();
        m_FencePool.reset();

        if (m_MemoryAllocator != VK_NULL_HANDLE)
        {
            VmaStats stats;
            vmaCalculateStats(m_MemoryAllocator, &stats);

            ENG_CORE_INFO("Total device memory leaked: {} bytes.", stats.total.usedBytes);
            vmaDestroyAllocator(m_MemoryAllocator);
        }

        if (m_Handle != VK_NULL_HANDLE)
            vkDestroyDevice(m_Handle, nullptr);
    }

    VkResult Device::WaitIdle()
    {
        return VK_SUCCESS;
    }

    bool Device::IsExtensionEnabled(const char *extension)
    {
        return std::find_if(m_EnabledExtensions.begin(), m_EnabledExtensions.end(), [extension](const char *enabled_extension)
                            { return std::strcmp(extension, enabled_extension) == 0; }) != m_EnabledExtensions.end();
    }

    bool Device::IsExtensionSupported(const char *requested_extension) const
    {
        auto it = std::find_if(m_DeviceExtensions.begin(),
                               m_DeviceExtensions.end(),
                               [requested_extension](auto &enabled_extension)
                               {
                                   return (std::strcmp(enabled_extension.extensionName, requested_extension) == 0);
                               });

        return it != m_DeviceExtensions.end();
    }

    const QueueFamily &Device::GetSuitableGraphicsQueueFamily()
    {
        for (auto &queue_family : m_QueueFamilies)
        {
            if (queue_family.CanPresent() &&
                queue_family.GetQueues().size() > 0)
                return queue_family;
        }

        return GetQueueFamilyByFlags(VK_QUEUE_GRAPHICS_BIT);
    }

    QueueFamily &Device::GetQueueFamilyByFlags(VkQueueFlags required_queue_flags)
    {
        size_t queue_family_index = 0;
        for (auto &queue_family : m_QueueFamilies)
        {
            VkQueueFlags queue_flags = queue_family.GetProperties().queueFlags;

            if ((queue_flags & required_queue_flags) == required_queue_flags)
                return m_QueueFamilies[queue_family_index];

            queue_family_index++;
        }

        throw std::runtime_error("Queue not found");
    }
}