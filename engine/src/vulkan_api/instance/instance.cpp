#include "vulkan_api/instance/instance.h"

#include "common/base_common.h"
#include "common/error_common.h"

namespace
{
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                               VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                               const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                               void *user_data)
    {
        switch (message_severity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            ENG_VULKAN_TRACE("{}", callback_data->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            ENG_VULKAN_WARN("{}", callback_data->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            ENG_VULKAN_ERROR("{}", callback_data->pMessage);
            break;

        default:
            break;
        }

        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,
                                                        VkDebugReportObjectTypeEXT type,
                                                        uint64_t object,
                                                        size_t location,
                                                        int32_t message_code,
                                                        const char *layer_prefix,
                                                        const char *message,
                                                        void *user_data)
    {
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            ENG_VULKAN_ERROR("{}: {}", layer_prefix, message);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            ENG_VULKAN_WARN("{}: {}", layer_prefix, message);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            ENG_VULKAN_WARN("{}: {}", layer_prefix, message);
        }
        else
        {
            ENG_VULKAN_INFO("{}: {}", layer_prefix, message);
        }
        return VK_FALSE;
    }

    bool ValidateLayers(const std::vector<const char *> &required,
                        const std::vector<VkLayerProperties> &available)
    {
        for (auto layer : required)
        {
            bool found = false;
            for (auto &available_layer : available)
            {
                if (strcmp(available_layer.layerName, layer) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                ENG_CORE_ERROR("Validation Layer {} not found", layer);
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> GetOptimalValidationLayers(const std::vector<VkLayerProperties> &supported_instance_layers)
    {
        std::vector<std::vector<const char *>> validation_layer_priority_list =
            {
                // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
                {"VK_LAYER_KHRONOS_validation"},

                // Otherwise we fallback to using the LunarG meta layer
                {"VK_LAYER_LUNARG_standard_validation"},

                // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
                {
                    "VK_LAYER_GOOGLE_threading",
                    "VK_LAYER_LUNARG_parameter_validation",
                    "VK_LAYER_LUNARG_object_tracker",
                    "VK_LAYER_LUNARG_core_validation",
                    "VK_LAYER_GOOGLE_unique_objects",
                },

                // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
                {"VK_LAYER_LUNARG_core_validation"}};

        for (auto &validation_layers : validation_layer_priority_list)
        {
            if (ValidateLayers(validation_layers, supported_instance_layers))
            {
                return validation_layers;
            }

            ENG_CORE_WARN("Couldn't enable validation layers (see log for error) - falling back");
        }

        // Else return nothing
        return {};
    }
}

namespace engine
{
    Instance::Instance(std::string &name,
                       std::unordered_map<const char *, bool> &required_extensions,
                       std::vector<const char *> &required_validation_layers,
                       DebugUtilsSettings debug_messenger_settings,
                       bool headless,
                       uint32_t api_version)
        : m_DebugUtilsSettings(debug_messenger_settings)
    {
        VkResult result = volkInitialize();
        if (result)
            throw VulkanException(result, "Failed to initialize volk.");

        uint32_t instance_extension_count;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
        std::vector<VkExtensionProperties> available_instance_extensions(instance_extension_count);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, available_instance_extensions.data()));

#if defined(ENG_DEBUG) || defined(ENG_VALIDATION_LAYERS)
        bool debug_utils = false;
        for (auto &available_extension : available_instance_extensions)
        {
            if (strcmp(available_extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                m_EnabledExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                debug_utils = true;
                ENG_CORE_TRACE("{} is available, enabling it", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
        }

        if (!debug_utils)
        {
            m_EnabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
#endif

#if (defined(ENG_DEBUG) || defined(ENG_VALIDATION_LAYERS)) && defined(ENG_VALIDATION_LAYERS_GPU_ASSISTED)
        bool validation_features = false;
        {
            uint32_t layer_instance_extension_count;
            VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layer_instance_extension_count, nullptr));

            std::vector<VkExtensionProperties> available_layer_instance_extensions(layer_instance_extension_count);
            VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layer_instance_extension_count, available_layer_instance_extensions.data()));

            for (auto &available_extension : available_layer_instance_extensions)
            {
                if (strcmp(available_extension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0)
                {
                    m_EnabledExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
                    validation_features = true;
                    ENG_CORE_TRACE("{} is available, enabling it", VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
                }
            }
        }
#endif

        if (headless)
        {
            bool headless_extension = false;
            for (auto &available_extension : available_instance_extensions)
            {
                if (strcmp(available_extension.extensionName, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME) == 0)
                {
                    m_EnabledExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
                    headless_extension = true;
                    ENG_CORE_TRACE("{} is available, enabling it", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
                }
            }
            if (!headless_extension)
            {
                ENG_CORE_TRACE("{} is not available, disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
            }
        }
        else
        {
            m_EnabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        }

        bool extension_found = false;
        for (auto &extension : required_extensions)
        {
            extension_found = false;
            const char *extension_name = extension.first;
            bool is_optional = extension.second;

            for (int i = 0; i < available_instance_extensions.size(); i++)
            {
                if (strcmp(extension_name, available_instance_extensions[i].extensionName) == 0)
                {
                    extension_found = true;
                    m_EnabledExtensions.emplace_back(extension_name);
                    break;
                }
            }

            if (!extension_found)
            {
                if (is_optional)
                    ENG_CORE_ERROR("Optional instance extension {} not available, some features may be disabled.", extension_name);
                else
                {
                    ENG_CORE_CRITICAL("Required instance extension {} not available, cannot run.", extension_name);
                    throw std::runtime_error("Required instance extensions are missing.\n");
                }
            }
        }

        uint32_t validation_layer_count;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr));
        std::vector<VkLayerProperties> supported_validation_layers(validation_layer_count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&validation_layer_count, supported_validation_layers.data()));

        std::vector<const char *> requested_validation_layers(required_validation_layers);

#ifdef ENG_VALIDATION_LAYERS
        std::vector<const char *> optimal_validation_layers = GetOptimalValidationLayers(supported_validation_layers);
        requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());
#endif

        if (ValidateLayers(requested_validation_layers, supported_validation_layers))
        {
            ENG_CORE_TRACE("Enabled Validation Layers:");
            for (const auto &layer : requested_validation_layers)
                ENG_CORE_TRACE("	\t{}", layer);
        }
        else
            throw std::runtime_error("Required validation layers are missing.");

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = name.c_str();
        app_info.applicationVersion = 0;
        app_info.pEngineName = "Engine";
        app_info.engineVersion = 0;
        app_info.apiVersion = api_version;

        VkInstanceCreateInfo instance_info;
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;
        instance_info.flags = VK_FLAGS_NONE;

        instance_info.enabledExtensionCount = to_u32(m_EnabledExtensions.size());
        instance_info.ppEnabledExtensionNames = m_EnabledExtensions.data();

        instance_info.enabledLayerCount = to_u32(requested_validation_layers.size());
        instance_info.ppEnabledLayerNames = requested_validation_layers.data();

#if defined(ENG_DEBUG) || defined(ENG_VALIDATION_LAYERS)
        VkDebugUtilsMessengerCreateInfoEXT debug_utils_create_info{};
        VkDebugReportCallbackCreateInfoEXT debug_report_create_info{};

        if (debug_utils)
        {
            debug_utils_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug_utils_create_info.messageSeverity = m_DebugUtilsSettings.VulkanMessageSeverity;
            debug_utils_create_info.messageType = m_DebugUtilsSettings.VulkanMessageType;
            debug_utils_create_info.pfnUserCallback = DebugUtilsMessengerCallback;

            instance_info.pNext = &debug_utils_create_info;
        }
        else
        {
            debug_report_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            debug_report_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debug_report_create_info.pfnCallback = DebugCallback;

            instance_info.pNext = &debug_report_create_info;
        }
#endif

#if (defined(ENG_DEBUG) || defined(ENG_VALIDATION_LAYERS)) && defined(ENG_VALIDATION_LAYERS_GPU_ASSISTED)
        VkValidationFeaturesEXT validation_features_info{};

        if (validation_features)
        {
            const std::array<VkValidationFeatureEnableEXT, 2> enable_features = {
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            };

            validation_features_info.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            validation_features_info.enabledValidationFeatureCount = enable_features.size();
            validation_features_info.pEnabledValidationFeatures = enable_features.data();
            validation_features_info.pNext = instance_info.pNext;

            instance_info.pNext = &validation_features_info;
        }
#endif

        result = vkCreateInstance(&instance_info, nullptr, &m_Handle);

        if (result != VK_SUCCESS)
            throw VulkanException(result, "Could not create Vulkan instance");

        volkLoadInstance(m_Handle);

#if defined(ENG_DEBUG) || defined(ENG_VALIDATION_LAYERS)
        if (debug_utils)
        {
            result = vkCreateDebugUtilsMessengerEXT(m_Handle, &debug_utils_create_info, nullptr, &m_DebugUtilsMessenger);
            if (result != VK_SUCCESS)
                throw VulkanException(result, "Could not create debug utils messenger");
        }
        else
        {
            result = vkCreateDebugReportCallbackEXT(m_Handle, &debug_report_create_info, nullptr, &m_DebugReportCallback);
            if (result != VK_SUCCESS)
                throw VulkanException(result, "Could not create debug report callback");
        }
#endif
    }

    Instance::~Instance()
    {
#if defined(ENG_DEBUG) || defined(ENG_VALIDATION_LAYERS)
        if (m_DebugUtilsMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(m_Handle, m_DebugUtilsMessenger, nullptr);
        }
        if (m_DebugReportCallback != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(m_Handle, m_DebugReportCallback, nullptr);
        }
#endif

        if (m_Handle != VK_NULL_HANDLE)
        {
            vkDestroyInstance(m_Handle, nullptr);
        }
    }
}