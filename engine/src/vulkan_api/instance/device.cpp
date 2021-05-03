#include "vulkan_api/instance/device.h"

#include "common/error_common.h"

namespace engine
{
    Device::Device(PhysicalDevice &gpu,
                   VkSurfaceKHR surface,
                   std::unordered_map<const char *, bool> requested_extensions)
        : m_Gpu(gpu), m_ResourceCache(*this)
    {
        ENG_CORE_TRACE("Selected GPU: {}", gpu.GetProperties().deviceName);

        std::vector<VkQueueFamilyProperties> queue_family_properties = gpu.GetQueueFamilyProperties();
        uint32_t queue_family_properties_count = to_u32(queue_family_properties.size());

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

            ENG_CORE_TRACE("Dedicated Allocation enabled");
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
            ENG_CORE_TRACE("Device supports the following requested extensions:");
            for (auto &extension : m_EnabledExtensions)
            {
                ENG_CORE_TRACE("  \t{}", extension);
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
        create_info.queueCreateInfoCount = to_u32(queue_create_infos.size());
        create_info.ppEnabledExtensionNames = m_EnabledExtensions.data();
        create_info.enabledExtensionCount = to_u32(m_EnabledExtensions.size());

        const auto requested_gpu_features = gpu.GetRequestedFeatures();
        create_info.pEnabledFeatures = &requested_gpu_features;

        VkResult result = vkCreateDevice(gpu.GetHandle(), &create_info, nullptr, &m_Handle);

        if (result != VK_SUCCESS)
            throw VulkanException(result, "Cannot create device");
    }

    Device::~Device()
    {
    }

    VkResult Device::WaitIdle()
    {
        return VK_SUCCESS;
    }

    bool Device::IsExtensionSupported(const char *requested_extension) const
    {
        auto it = std::find_if(m_DeviceExtensions.begin(),
                               m_DeviceExtensions.end(),
                               [requested_extension](auto &enabled_extension) {
                                   return (std::strcmp(enabled_extension.extensionName, requested_extension) == 0);
                               });

        return it != m_DeviceExtensions.end();
    }
}