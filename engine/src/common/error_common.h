#pragma once

#include "common/vulkan_common.h"
#include "common/strings_common.h"
#include <stdexcept>

namespace engine
{
    class VulkanException : public std::runtime_error
    {
    public:
        VulkanException(VkResult result, const std::string &message = "Vulkan error");

        const char *what() const noexcept override;
        VkResult result;

    private:
        std::string error_message;
    };
}

#define VK_CHECK(x)                                                             \
    do                                                                          \
    {                                                                           \
        VkResult err = x;                                                       \
        if (err)                                                                \
        {                                                                       \
            ENG_CORE_ERROR("Detected Vulkan error: {}", engine::ToString(err)); \
            abort();                                                            \
        }                                                                       \
    } while (0)

#define ASSERT_VK_HANDLE(handle)                 \
    do                                           \
    {                                            \
        if ((handle) == VK_NULL_HANDLE)          \
        {                                        \
            ENG_CORE_CRITICAL("Handle is NULL"); \
            abort();                             \
        }                                        \
    } while (0)