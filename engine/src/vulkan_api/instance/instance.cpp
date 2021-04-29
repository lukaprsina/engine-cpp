#include "vulkan_api/instance/instance.h"

#include "common/vulkan_common.h"
#include "common/error_common.h"

namespace engine
{
    Instance::Instance(std::string &name,
                       std::unordered_map<const char *, bool> &instance_extension,
                       std::vector<const char *> &validation_layers,
                       bool headless,
                       uint32_t api_version)
    {
        VkResult result = volkInitialize();
        if (result)
            throw VulkanException(result, "Failed to initialize volk.");

        uint32_t instance_extension_count;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
        std::vector<VkExtensionProperties> available_instance_extensions(instance_extension_count);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, available_instance_extensions.data()));
    }

    Instance::~Instance()
    {
    }
}